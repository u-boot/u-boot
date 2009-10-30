/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <netdev.h>
#include "mv88e61xx.h"

#ifdef CONFIG_MV88E61XX_MULTICHIP_ADRMODE
/* Chip Address mode
 * The Switch support two modes of operation
 * 1. single chip mode and
 * 2. Multi-chip mode
 * Refer section 9.2 &9.3 in chip datasheet-02 for more details
 *
 * By default single chip mode is configured
 * multichip mode operation can be configured in board header
 */
static int mv88e61xx_busychk_multic(char *name, u32 devaddr)
{
	u16 reg = 0;
	u32 timeout = MV88E61XX_PHY_TIMEOUT;

	/* Poll till SMIBusy bit is clear */
	do {
		miiphy_read(name, devaddr, 0x0, &reg);
		if (timeout-- == 0) {
			printf("SMI busy timeout\n");
			return -1;
		}
	} while (reg & (1 << 15));
	return 0;
}

static void mv88e61xx_wr_phy(char *name, u32 phy_adr, u32 reg_ofs, u16 data)
{
	u16 mii_dev_addr;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, &mii_dev_addr)) {
		printf("Error..could not read PHY dev address\n");
		return;
	}
	mv88e61xx_busychk_multic(name, mii_dev_addr);
	/* Write data to Switch indirect data register */
	miiphy_write(name, mii_dev_addr, 0x1, data);
	/* Write command to Switch indirect command register (write) */
	miiphy_write(name, mii_dev_addr, 0x0,
		     reg_ofs | (phy_adr << 5) | (1 << 10) | (1 << 12) | (1 <<
									 15));
}

static void mv88e61xx_rd_phy(char *name, u32 phy_adr, u32 reg_ofs, u16 * data)
{
	u16 mii_dev_addr;

	/* command to read PHY dev address */
	if (miiphy_read(name, 0xEE, 0xEE, &mii_dev_addr)) {
		printf("Error..could not read PHY dev address\n");
		return;
	}
	mv88e61xx_busychk_multic(name, mii_dev_addr);
	/* Write command to Switch indirect command register (read) */
	miiphy_write(name, mii_dev_addr, 0x0,
		     reg_ofs | (phy_adr << 5) | (1 << 11) | (1 << 12) | (1 <<
									 15));
	mv88e61xx_busychk_multic(name, mii_dev_addr);
	/* Read data from Switch indirect data register */
	miiphy_read(name, mii_dev_addr, 0x1, data);
}
#endif /* CONFIG_MV88E61XX_MULTICHIP_ADRMODE */

static void mv88e61xx_port_vlan_config(struct mv88e61xx_config *swconfig,
				       u32 max_prtnum, u32 ports_ofs)
{
	u32 prt;
	u16 reg;
	char *name = swconfig->name;
	u32 cpu_port = swconfig->cpuport;
	u32 port_mask = swconfig->ports_enabled;
	enum mv88e61xx_cfg_vlan vlancfg = swconfig->vlancfg;

	/* be sure all ports are disabled */
	for (prt = 0; prt < max_prtnum; prt++) {
		RD_PHY(name, ports_ofs + prt, MV88E61XX_PRT_CTRL_REG, &reg);
		reg &= ~0x3;
		WR_PHY(name, ports_ofs + prt, MV88E61XX_PRT_CTRL_REG, reg);

		if (!(cpu_port & (1 << prt)))
			continue;
		/* Set CPU port VID to 0x1 */
		RD_PHY(name, (ports_ofs + prt), MV88E61XX_PRT_VID_REG, &reg);
		reg &= ~0xfff;
		reg |= 0x1;
		WR_PHY(name, (ports_ofs + prt), MV88E61XX_PRT_VID_REG, reg);
	}

	/* Setting  Port default priority for all ports to zero */
	for (prt = 0; prt < max_prtnum; prt++) {
		RD_PHY(name, ports_ofs + prt, MV88E61XX_PRT_VID_REG, &reg);
		reg &= ~0xc000;
		WR_PHY(name, ports_ofs + prt, MV88E61XX_PRT_VID_REG, reg);
	}
	/* Setting VID and VID map for all ports except CPU port */
	for (prt = 0; prt < max_prtnum; prt++) {
		/* only for enabled ports */
		if ((1 << prt) & port_mask) {
			/* skip CPU port */
			if ((1 << prt) & cpu_port) {
				/*
				 * Set Vlan map table for cpu_port to see
				 * all ports
				 */
				RD_PHY(name, (ports_ofs + prt),
				       MV88E61XX_PRT_VMAP_REG, &reg);
				reg &= ~((1 << max_prtnum) - 1);
				reg |= port_mask & ~(1 << prt);
				WR_PHY(name, (ports_ofs + prt),
				       MV88E61XX_PRT_VMAP_REG, reg);
			} else {

				/*
				 *  set Ports VLAN Mapping.
				 *      port prt <--> cpu_port VLAN #prt+1.
				 */
				RD_PHY(name, ports_ofs + prt,
				       MV88E61XX_PRT_VID_REG, &reg);
				reg &= ~0x0fff;
				reg |= (prt + 1);
				WR_PHY(name, ports_ofs + prt,
				       MV88E61XX_PRT_VID_REG, reg);

				RD_PHY(name, ports_ofs + prt,
				       MV88E61XX_PRT_VMAP_REG, &reg);
				if (vlancfg == MV88E61XX_VLANCFG_DEFAULT) {
					/*
					 * all any port can send frames to all other ports
					 * ref: sec 3.2.1.1 of datasheet
					 */
					reg |= 0x03f;
					reg &= ~(1 << prt);
				} else if (vlancfg == MV88E61XX_VLANCFG_ROUTER) {
					/*
					 * all other ports can send frames to CPU port only
					 * ref: sec 3.2.1.2 of datasheet
					 */
					reg &= ~((1 << max_prtnum) - 1);
					reg |= cpu_port;
				}
				WR_PHY(name, ports_ofs + prt,
				       MV88E61XX_PRT_VMAP_REG, reg);
			}
		}
	}

	/*
	 * enable only appropriate ports to forwarding mode
	 * and disable the others
	 */
	for (prt = 0; prt < max_prtnum; prt++) {
		if ((1 << prt) & port_mask) {
			RD_PHY(name, ports_ofs + prt,
			       MV88E61XX_PRT_CTRL_REG, &reg);
			reg |= 0x3;
			WR_PHY(name, ports_ofs + prt,
			       MV88E61XX_PRT_CTRL_REG, reg);
		} else {
			/* Disable port */
			RD_PHY(name, ports_ofs + prt,
			       MV88E61XX_PRT_CTRL_REG, &reg);
			reg &= ~0x3;
			WR_PHY(name, ports_ofs + prt,
			       MV88E61XX_PRT_CTRL_REG, reg);
		}
	}
}

/*
 * Make sure SMIBusy bit cleared before another
 * SMI operation can take place
 */
static int mv88e61xx_busychk(char *name)
{
	u16 reg = 0;
	u32 timeout = MV88E61XX_PHY_TIMEOUT;
	do {
		RD_PHY(name, MV88E61XX_GLB2REG_DEVADR,
		       MV88E61XX_PHY_CMD, &reg);
		if (timeout-- == 0) {
			printf("SMI busy timeout\n");
			return -1;
		}
	} while (reg & 1 << 15);	/* busy mask */
	return 0;
}

/*
 * Power up the specified port and reset PHY
 */
static int mv88361xx_powerup(struct mv88e61xx_config *swconfig, u32 prt)
{
	char *name = swconfig->name;

	/* Write Copper Specific control reg1 (0x14) for-
	 * Enable Phy power up
	 * Energy Detect on (sense&Xmit NLP Periodically
	 * reset other settings default
	 */
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, 0x3360);
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR,
	       MV88E61XX_PHY_CMD, (0x9410 | (prt << 5)));

	if (mv88e61xx_busychk(name))
		return -1;

	/* Write PHY ctrl reg (0x0) to apply
	 * Phy reset (set bit 15 low)
	 * reset other default values
	 */
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, 0x1140);
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR,
	       MV88E61XX_PHY_CMD, (0x9400 | (prt << 5)));

	if (mv88e61xx_busychk(name))
		return -1;

	return 0;
}

/*
 * Default Setup for LED[0]_Control (ref: Table 46 Datasheet-3)
 * is set to "On-1000Mb/s Link, Off Else"
 * This function sets it to "On-Link, Blink-Activity, Off-NoLink"
 *
 * This is optional settings may be needed on some boards
 * to setup PHY LEDs default configuration to detect 10/100/1000Mb/s
 * Link status
 */
static int mv88361xx_led_init(struct mv88e61xx_config *swconfig, u32 prt)
{
	char *name = swconfig->name;
	u16 reg;

	if (swconfig->led_init != MV88E61XX_LED_INIT_EN)
		return 0;

	/* set page address to 3 */
	reg = 3;
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, reg);
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR,
	       MV88E61XX_PHY_CMD, (1 << MV88E61XX_BUSY_OFST |
				   1 << MV88E61XX_MODE_OFST |
				   1 << MV88E61XX_OP_OFST |
				   prt << MV88E61XX_ADDR_OFST | 22));

	if (mv88e61xx_busychk(name))
		return -1;

	/* set LED Func Ctrl reg */
	reg = 1;	/* LED[0] On-Link, Blink-Activity, Off-NoLink */
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, reg);
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR,
	       MV88E61XX_PHY_CMD, (1 << MV88E61XX_BUSY_OFST |
				   1 << MV88E61XX_MODE_OFST |
				   1 << MV88E61XX_OP_OFST |
				   prt << MV88E61XX_ADDR_OFST | 16));

	if (mv88e61xx_busychk(name))
		return -1;

	/* set page address to 0 */
	reg = 0;
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, reg);
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR,
	       MV88E61XX_PHY_CMD, (1 << MV88E61XX_BUSY_OFST |
				   1 << MV88E61XX_MODE_OFST |
				   1 << MV88E61XX_OP_OFST |
				   prt << MV88E61XX_ADDR_OFST | 22));

	if (mv88e61xx_busychk(name))
		return -1;

	return 0;
}

/*
 * Reverse Transmit polarity for Media Dependent Interface
 * Pins (MDIP) bits in Copper Specific Control Register 3
 * (Page 0, Reg 20 for each phy (except cpu port)
 * Reference: Section 1.1 Switch datasheet-3
 *
 * This is optional settings may be needed on some boards
 * for PHY<->magnetics h/w tuning
 */
static int mv88361xx_reverse_mdipn(struct mv88e61xx_config *swconfig, u32 prt)
{
	char *name = swconfig->name;
	u16 reg;

	if (swconfig->mdip != MV88E61XX_MDIP_REVERSE)
		return 0;

	reg = 0x0f;		/*Reverse MDIP/N[3:0] bits */
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, reg);
	WR_PHY(name, MV88E61XX_GLB2REG_DEVADR,
	       MV88E61XX_PHY_CMD, (1 << MV88E61XX_BUSY_OFST |
				   1 << MV88E61XX_MODE_OFST |
				   1 << MV88E61XX_OP_OFST |
				   prt << MV88E61XX_ADDR_OFST | 20));

	if (mv88e61xx_busychk(name))
		return -1;

	return 0;
}

/*
 * Marvell 88E61XX Switch initialization
 */
int mv88e61xx_switch_initialize(struct mv88e61xx_config *swconfig)
{
	u32 prt;
	u16 reg;
	char *idstr;
	char *name = swconfig->name;

	if (miiphy_set_current_dev(name)) {
		printf("%s failed\n", __FUNCTION__);
		return -1;
	}

	if (!(swconfig->cpuport & ((1 << 4) | (1 << 5)))) {
		swconfig->cpuport = (1 << 5);
		printf("Invalid cpu port config, using default port5\n");
	}

	RD_PHY(name, MV88E61XX_PRT_OFST, PHY_PHYIDR2, &reg);
	switch (reg &= 0xfff0) {
	case 0x1610:
		idstr = "88E6161";
		break;
	case 0x1650:
		idstr = "88E6165";
		break;
	case 0x1210:
		idstr = "88E6123";
		/* ports 2,3,4 not available */
		swconfig->ports_enabled &= 0x023;
		break;
	default:
		/* Could not detect switch id */
		idstr = "88E61??";
		break;
	}

	/* Port based VLANs configuration */
	if ((swconfig->vlancfg == MV88E61XX_VLANCFG_DEFAULT)
	    || (swconfig->vlancfg == MV88E61XX_VLANCFG_ROUTER))
		mv88e61xx_port_vlan_config(swconfig, MV88E61XX_MAX_PORTS_NUM,
					   MV88E61XX_PRT_OFST);
	else {
		printf("Unsupported mode %s failed\n", __FUNCTION__);
		return -1;
	}

	if (swconfig->rgmii_delay == MV88E61XX_RGMII_DELAY_EN) {
		/*
		 * Enable RGMII delay on Tx and Rx for CPU port
		 * Ref: sec 9.5 of chip datasheet-02
		 */
		WR_PHY(name, MV88E61XX_PRT_OFST + 5,
		       MV88E61XX_RGMII_TIMECTRL_REG, 0x18);
		WR_PHY(name, MV88E61XX_PRT_OFST + 4,
		       MV88E61XX_RGMII_TIMECTRL_REG, 0xc1e7);
	}

	for (prt = 0; prt < MV88E61XX_MAX_PORTS_NUM; prt++) {
		if (!((1 << prt) & swconfig->cpuport)) {

			if (mv88361xx_led_init(swconfig, prt))
				return -1;
			if (mv88361xx_reverse_mdipn(swconfig, prt))
				return -1;
			if (mv88361xx_powerup(swconfig, prt))
				return -1;
		}

		/*Program port state */
		RD_PHY(name, MV88E61XX_PRT_OFST + prt,
		       MV88E61XX_PRT_CTRL_REG, &reg);
		WR_PHY(name, MV88E61XX_PRT_OFST + prt,
		       MV88E61XX_PRT_CTRL_REG,
		       reg | (swconfig->portstate & 0x03));
	}

	printf("%s Initialized on %s\n", idstr, name);
	return 0;
}

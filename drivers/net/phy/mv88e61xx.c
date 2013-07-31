/*
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <netdev.h>
#include "mv88e61xx.h"

/*
 * Uncomment either of the following line for local debug control;
 * otherwise global debug control will apply.
 */

/* #undef DEBUG */
/* #define DEBUG */

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

static void mv88e61xx_switch_write(char *name, u32 phy_adr,
	u32 reg_ofs, u16 data)
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

static void mv88e61xx_switch_read(char *name, u32 phy_adr,
	u32 reg_ofs, u16 *data)
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

/*
 * Convenience macros for switch device/port reads/writes
 * These macros output valid 'mv88e61xx' U_BOOT_CMDs
 */

#ifndef DEBUG
#define WR_SWITCH_REG wr_switch_reg
#define RD_SWITCH_REG rd_switch_reg
#define WR_SWITCH_PORT_REG(n, p, r, d) \
	WR_SWITCH_REG(n, (MV88E61XX_PRT_OFST+p), r, d)
#define RD_SWITCH_PORT_REG(n, p, r, d) \
	RD_SWITCH_REG(n, (MV88E61XX_PRT_OFST+p), r, d)
#else
static void WR_SWITCH_REG(char *name, u32 dev_adr, u32 reg_ofs, u16 data)
{
	printf("mv88e61xx %s dev %02x reg %02x write %04x\n",
		name, dev_adr, reg_ofs, data);
	wr_switch_reg(name, dev_adr, reg_ofs, data);
}
static void RD_SWITCH_REG(char *name, u32 dev_adr, u32 reg_ofs, u16 *data)
{
	rd_switch_reg(name, dev_adr, reg_ofs, data);
	printf("mv88e61xx %s dev %02x reg %02x read %04x\n",
		name, dev_adr, reg_ofs, *data);
}
static void WR_SWITCH_PORT_REG(char *name, u32 prt_adr, u32 reg_ofs,
	u16 data)
{
	printf("mv88e61xx %s port %02x reg %02x write %04x\n",
		name, prt_adr, reg_ofs, data);
	wr_switch_reg(name, (MV88E61XX_PRT_OFST+prt_adr), reg_ofs, data);
}
static void RD_SWITCH_PORT_REG(char *name, u32 prt_adr, u32 reg_ofs,
	u16 *data)
{
	rd_switch_reg(name, (MV88E61XX_PRT_OFST+prt_adr), reg_ofs, data);
	printf("mv88e61xx %s port %02x reg %02x read %04x\n",
		name, prt_adr, reg_ofs, *data);
}
#endif

/*
 * Local functions to read/write registers on the switch PHYs.
 * NOTE! This goes through switch, not direct miiphy, writes and reads!
 */

/*
 * Make sure SMIBusy bit cleared before another
 * SMI operation can take place
 */
static int mv88e61xx_busychk(char *name)
{
	u16 reg = 0;
	u32 timeout = MV88E61XX_PHY_TIMEOUT;
	do {
		rd_switch_reg(name, MV88E61XX_GLB2REG_DEVADR,
		       MV88E61XX_PHY_CMD, &reg);
		if (timeout-- == 0) {
			printf("SMI busy timeout\n");
			return -1;
		}
	} while (reg & 1 << 15);	/* busy mask */
	return 0;
}

static inline int mv88e61xx_switch_miiphy_write(char *name, u32 phy,
	u32 reg, u16 data)
{
	/* write switch data reg then cmd reg then check completion */
	wr_switch_reg(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA,
		data);
	wr_switch_reg(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_CMD,
		(MV88E61XX_PHY_WRITE_CMD | (phy << 5)  | reg));
	return mv88e61xx_busychk(name);
}

static inline int mv88e61xx_switch_miiphy_read(char *name, u32 phy,
	u32 reg, u16 *data)
{
	/* write switch cmd reg, check for completion */
	wr_switch_reg(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_CMD,
		(MV88E61XX_PHY_READ_CMD | (phy << 5)  | reg));
	if (mv88e61xx_busychk(name))
		return -1;
	/* read switch data reg and return success */
	rd_switch_reg(name, MV88E61XX_GLB2REG_DEVADR, MV88E61XX_PHY_DATA, data);
	return 0;
}

/*
 * Convenience macros for switch PHY reads/writes
 */

#ifndef DEBUG
#define WR_SWITCH_PHY_REG mv88e61xx_switch_miiphy_write
#define RD_SWITCH_PHY_REG mv88e61xx_switch_miiphy_read
#else
static inline int WR_SWITCH_PHY_REG(char *name, u32 phy_adr,
	u32 reg_ofs, u16 data)
{
	int r = mv88e61xx_switch_miiphy_write(name, phy_adr, reg_ofs, data);
	if (r)
		printf("** ERROR writing mv88e61xx %s phy %02x reg %02x\n",
			name, phy_adr, reg_ofs);
	else
		printf("mv88e61xx %s phy %02x reg %02x write %04x\n",
			name, phy_adr, reg_ofs, data);
	return r;
}
static inline int RD_SWITCH_PHY_REG(char *name, u32 phy_adr,
	u32 reg_ofs, u16 *data)
{
	int r = mv88e61xx_switch_miiphy_read(name, phy_adr, reg_ofs, data);
	if (r)
		printf("** ERROR reading mv88e61xx %s phy %02x reg %02x\n",
			name, phy_adr, reg_ofs);
	else
		printf("mv88e61xx %s phy %02x reg %02x read %04x\n",
			name, phy_adr, reg_ofs, *data);
	return r;
}
#endif

static void mv88e61xx_port_vlan_config(struct mv88e61xx_config *swconfig)
{
	u32 prt;
	u16 reg;
	char *name = swconfig->name;
	u32 port_mask = swconfig->ports_enabled;

	/* apply internal vlan config */
	for (prt = 0; prt < MV88E61XX_MAX_PORTS_NUM; prt++) {
		/* only for enabled ports */
		if ((1 << prt) & port_mask) {
			/* take vlan map from swconfig */
			u8 vlanmap = swconfig->vlancfg[prt];
			/* remove disabled ports from vlan map */
			vlanmap &= swconfig->ports_enabled;
			/* apply vlan map to port */
			RD_SWITCH_PORT_REG(name, prt,
				MV88E61XX_PRT_VMAP_REG, &reg);
			reg &= ~((1 << MV88E61XX_MAX_PORTS_NUM) - 1);
			reg |= vlanmap;
			WR_SWITCH_PORT_REG(name, prt,
				MV88E61XX_PRT_VMAP_REG, reg);
		}
	}
}

/*
 * Power up the specified port and reset PHY
 */
static int mv88361xx_powerup(struct mv88e61xx_config *swconfig, u32 phy)
{
	char *name = swconfig->name;

	/* Write Copper Specific control reg1 (0x10) for-
	 * Enable Phy power up
	 * Energy Detect on (sense&Xmit NLP Periodically
	 * reset other settings default
	 */
	if (WR_SWITCH_PHY_REG(name, phy, 0x10, 0x3360))
		return -1;

	/* Write PHY ctrl reg (0x0) to apply
	 * Phy reset (set bit 15 low)
	 * reset other default values
	 */
	if (WR_SWITCH_PHY_REG(name, phy, 0x00, 0x9140))
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
static int mv88361xx_led_init(struct mv88e61xx_config *swconfig, u32 phy)
{
	char *name = swconfig->name;

	if (swconfig->led_init != MV88E61XX_LED_INIT_EN)
		return 0;

	/* set page address to 3 */
	if (WR_SWITCH_PHY_REG(name, phy, 0x16, 0x0003))
		return -1;

	/*
	 * set LED Func Ctrl reg
	 * value 0x0001 = LED[0] On-Link, Blink-Activity, Off-NoLink
	 */
	if (WR_SWITCH_PHY_REG(name, phy, 0x10, 0x0001))
		return -1;

	/* set page address to 0 */
	if (WR_SWITCH_PHY_REG(name, phy, 0x16, 0x0000))
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
static int mv88361xx_reverse_mdipn(struct mv88e61xx_config *swconfig, u32 phy)
{
	char *name = swconfig->name;

	if (swconfig->mdip != MV88E61XX_MDIP_REVERSE)
		return 0;

	/*Reverse MDIP/N[3:0] bits */
	if (WR_SWITCH_PHY_REG(name, phy, 0x14, 0x000f))
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
	int time;

	if (miiphy_set_current_dev(name)) {
		printf("%s failed\n", __FUNCTION__);
		return -1;
	}

	if (!(swconfig->cpuport & ((1 << 4) | (1 << 5)))) {
		swconfig->cpuport = (1 << 5);
		printf("Invalid cpu port config, using default port5\n");
	}

	RD_SWITCH_PORT_REG(name, 0, MII_PHYSID2, &reg);
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

	/* be sure all ports are disabled */
	for (prt = 0; prt < MV88E61XX_MAX_PORTS_NUM; prt++) {
		RD_SWITCH_PORT_REG(name, prt, MV88E61XX_PRT_CTRL_REG, &reg);
		reg &= ~0x3;
		WR_SWITCH_PORT_REG(name, prt, MV88E61XX_PRT_CTRL_REG, reg);
	}

	/* wait 2 ms for queues to drain */
	udelay(2000);

	/* reset switch */
	RD_SWITCH_REG(name, MV88E61XX_GLBREG_DEVADR, MV88E61XX_SGCR, &reg);
	reg |= 0x8000;
	WR_SWITCH_REG(name, MV88E61XX_GLBREG_DEVADR, MV88E61XX_SGCR, reg);

	/* wait up to 1 second for switch reset complete */
	for (time = 1000; time; time--) {
		RD_SWITCH_REG(name, MV88E61XX_GLBREG_DEVADR, MV88E61XX_SGSR,
			&reg);
		if ((reg & 0xc800) == 0xc800)
			break;
		udelay(1000);
	}
	if (!time)
		return -1;

	/* Port based VLANs configuration */
	mv88e61xx_port_vlan_config(swconfig);

	if (swconfig->rgmii_delay == MV88E61XX_RGMII_DELAY_EN) {
		/*
		 * Enable RGMII delay on Tx and Rx for CPU port
		 * Ref: sec 9.5 of chip datasheet-02
		 */
		/*Force port link down */
		WR_SWITCH_PORT_REG(name, 5, MV88E61XX_PCS_CTRL_REG, 0x10);
		/* configure port RGMII delay */
		WR_SWITCH_PORT_REG(name, 4,
			MV88E61XX_RGMII_TIMECTRL_REG, 0x81e7);
		RD_SWITCH_PORT_REG(name, 5,
			MV88E61XX_RGMII_TIMECTRL_REG, &reg);
		WR_SWITCH_PORT_REG(name, 5,
			MV88E61XX_RGMII_TIMECTRL_REG, reg | 0x18);
		WR_SWITCH_PORT_REG(name, 4,
			MV88E61XX_RGMII_TIMECTRL_REG, 0xc1e7);
		/* Force port to RGMII FDX 1000Base then up */
		WR_SWITCH_PORT_REG(name, 5, MV88E61XX_PCS_CTRL_REG, 0x1e);
		WR_SWITCH_PORT_REG(name, 5, MV88E61XX_PCS_CTRL_REG, 0x3e);
	}

	for (prt = 0; prt < MV88E61XX_MAX_PORTS_NUM; prt++) {

		/* configure port's PHY */
		if (!((1 << prt) & swconfig->cpuport)) {
			/* port 4 has phy 6, not 4 */
			int phy = (prt == 4) ? 6 : prt;
			if (mv88361xx_powerup(swconfig, phy))
				return -1;
			if (mv88361xx_reverse_mdipn(swconfig, phy))
				return -1;
			if (mv88361xx_led_init(swconfig, phy))
				return -1;
		}

		/* set port VID to port+1 except for cpu port */
		if (!((1 << prt) & swconfig->cpuport)) {
			RD_SWITCH_PORT_REG(name, prt,
				MV88E61XX_PRT_VID_REG, &reg);
			WR_SWITCH_PORT_REG(name, prt,
				MV88E61XX_PRT_VID_REG,
				(reg & ~1023) | (prt+1));
		}

		/*Program port state */
		RD_SWITCH_PORT_REG(name, prt,
			MV88E61XX_PRT_CTRL_REG, &reg);
		WR_SWITCH_PORT_REG(name, prt,
			MV88E61XX_PRT_CTRL_REG,
			reg | (swconfig->portstate & 0x03));

	}

	printf("%s Initialized on %s\n", idstr, name);
	return 0;
}

#ifdef CONFIG_MV88E61XX_CMD
static int
do_switch(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *name, *endp;
	int write = 0;
	enum { dev, prt, phy } target = dev;
	u32 addrlo, addrhi, addr;
	u32 reglo, reghi, reg;
	u16 data, rdata;

	if (argc < 7)
		return -1;

	name = argv[1];

	if (strcmp(argv[2], "phy") == 0)
		target = phy;
	else if (strcmp(argv[2], "port") == 0)
		target = prt;
	else if (strcmp(argv[2], "dev") != 0)
		return 1;

	addrlo = simple_strtoul(argv[3], &endp, 16);

	if (!*endp) {
		addrhi = addrlo;
	} else {
		while (*endp < '0' || *endp > '9')
			endp++;
		addrhi = simple_strtoul(endp, NULL, 16);
	}

	reglo = simple_strtoul(argv[5], &endp, 16);
	if (!*endp) {
		reghi = reglo;
	} else {
		while (*endp < '0' || *endp > '9')
			endp++;
		reghi = simple_strtoul(endp, NULL, 16);
	}

	if (strcmp(argv[6], "write") == 0)
		write = 1;
	else if (strcmp(argv[6], "read") != 0)
		return 1;

	data = simple_strtoul(argv[7], NULL, 16);

	for (addr = addrlo; addr <= addrhi; addr++) {
		for (reg = reglo; reg <= reghi; reg++) {
			if (write) {
				if (target == phy)
					mv88e61xx_switch_miiphy_write(
						name, addr, reg, data);
				else if (target == prt)
					wr_switch_reg(name,
						addr+MV88E61XX_PRT_OFST,
						reg, data);
				else
					wr_switch_reg(name, addr, reg, data);
			} else {
				if (target == phy)
					mv88e61xx_switch_miiphy_read(
						name, addr, reg, &rdata);
				else if (target == prt)
					rd_switch_reg(name,
						addr+MV88E61XX_PRT_OFST,
						reg, &rdata);
				else
					rd_switch_reg(name, addr, reg, &rdata);
				printf("%s %s %s %02x %s %02x %s %04x\n",
					argv[0], argv[1], argv[2], addr,
					argv[4], reg, argv[6], rdata);
				if (write && argc == 7 && rdata != data)
					return 1;
			}
		}
	}
	return 0;
}

U_BOOT_CMD(mv88e61xx, 8, 0, do_switch,
	"Read or write mv88e61xx switch registers",
	"<ethdevice> dev|port|phy <addr> reg <reg> write <data>\n"
	"<ethdevice> dev|port|phy <addr> reg <reg> read [<data>]\n"
	"    - read/write switch device, port or phy at (addr,reg)\n"
	"      addr=0..0x1C for dev, 0..5 for port or phy.\n"
	"      reg=0..0x1F.\n"
	"      data=0..0xFFFF (tested if present against actual read).\n"
	"      All numeric parameters are assumed to be hex.\n"
	"      <addr> and <<reg> arguments can be ranges (x..y)"
);
#endif /* CONFIG_MV88E61XX_CMD */

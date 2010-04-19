/*
 * Intel LXT971/LXT972 PHY Driver for TI DaVinci
 * (TMS320DM644x) based boards.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * --------------------------------------------------------
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <net.h>
#include <miiphy.h>
#include <lxt971a.h>
#include <asm/arch/emac_defs.h>

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_CMD_NET

int lxt972_is_phy_connected(int phy_addr)
{
	u_int16_t id1, id2;

	if (!davinci_eth_phy_read(phy_addr, PHY_PHYIDR1, &id1))
		return(0);
	if (!davinci_eth_phy_read(phy_addr, PHY_PHYIDR2, &id2))
		return(0);

	if ((id1 == (0x0013)) && ((id2  & 0xfff0) == 0x78e0))
		return(1);

	return(0);
}

int lxt972_get_link_speed(int phy_addr)
{
	u_int16_t stat1, tmp;
	volatile emac_regs *emac = (emac_regs *)EMAC_BASE_ADDR;

	if (!davinci_eth_phy_read(phy_addr, PHY_LXT971_STAT2, &stat1))
		return(0);

	if (!(stat1 & PHY_LXT971_STAT2_LINK))	/* link up? */
		return(0);

	if (!davinci_eth_phy_read(phy_addr, PHY_LXT971_DIG_CFG, &tmp))
		return(0);

	tmp |= PHY_LXT971_DIG_CFG_MII_DRIVE;

	davinci_eth_phy_write(phy_addr, PHY_LXT971_DIG_CFG, tmp);
	/* Read back */
	if (!davinci_eth_phy_read(phy_addr, PHY_LXT971_DIG_CFG, &tmp))
		return(0);

	/* Speed doesn't matter, there is no setting for it in EMAC... */
	if (stat1 & PHY_LXT971_STAT2_DUPLEX_MODE) {
		/* set DM644x EMAC for Full Duplex  */
		emac->MACCONTROL = EMAC_MACCONTROL_MIIEN_ENABLE |
			EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
	} else {
		/*set DM644x EMAC for Half Duplex  */
		emac->MACCONTROL = EMAC_MACCONTROL_MIIEN_ENABLE;
	}

	return(1);
}


int lxt972_init_phy(int phy_addr)
{
	int ret = 1;

	if (!lxt972_get_link_speed(phy_addr)) {
		/* Try another time */
		ret = lxt972_get_link_speed(phy_addr);
	}

	/* Disable PHY Interrupts */
	davinci_eth_phy_write(phy_addr, PHY_LXT971_INT_ENABLE, 0);

	return(ret);
}


int lxt972_auto_negotiate(int phy_addr)
{
	u_int16_t tmp;

	if (!davinci_eth_phy_read(phy_addr, PHY_BMCR, &tmp))
		return(0);

	/* Restart Auto_negotiation  */
	tmp |= PHY_BMCR_RST_NEG;
	davinci_eth_phy_write(phy_addr, PHY_BMCR, tmp);

	/*check AutoNegotiate complete */
	udelay (10000);
	if (!davinci_eth_phy_read(phy_addr, PHY_BMSR, &tmp))
		return(0);

	if (!(tmp & PHY_BMSR_AUTN_COMP))
		return(0);

	return (lxt972_get_link_speed(phy_addr));
}

#endif	/* CONFIG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */

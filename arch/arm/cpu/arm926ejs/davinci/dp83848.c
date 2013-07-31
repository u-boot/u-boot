/*
 * National Semiconductor DP83848 PHY Driver for TI DaVinci
 * (TMS320DM644x) based boards.
 *
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * --------------------------------------------------------
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>
#include <dp83848.h>
#include <asm/arch/emac_defs.h>
#include "../../../../../drivers/net/davinci_emac.h"

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_CMD_NET

int dp83848_is_phy_connected(int phy_addr)
{
	u_int16_t	id1, id2;

	if (!davinci_eth_phy_read(phy_addr, DP83848_PHYID1_REG, &id1))
		return(0);
	if (!davinci_eth_phy_read(phy_addr, DP83848_PHYID2_REG, &id2))
		return(0);

	if ((id1 == DP83848_PHYID1_OUI) && (id2 == DP83848_PHYID2_OUI))
		return(1);

	return(0);
}

int dp83848_get_link_speed(int phy_addr)
{
	u_int16_t		tmp;
	volatile emac_regs*	emac = (emac_regs *)EMAC_BASE_ADDR;

	if (!davinci_eth_phy_read(phy_addr, DP83848_STAT_REG, &tmp))
		return(0);

	if (!(tmp & DP83848_LINK_STATUS))	/* link up? */
		return(0);

	if (!davinci_eth_phy_read(phy_addr, DP83848_PHY_STAT_REG, &tmp))
		return(0);

	/* Speed doesn't matter, there is no setting for it in EMAC... */
	if (tmp & DP83848_DUPLEX) {
		/* set DM644x EMAC for Full Duplex  */
		emac->MACCONTROL = EMAC_MACCONTROL_MIIEN_ENABLE |
			EMAC_MACCONTROL_FULLDUPLEX_ENABLE;
	} else {
		/*set DM644x EMAC for Half Duplex  */
		emac->MACCONTROL = EMAC_MACCONTROL_MIIEN_ENABLE;
	}

	return(1);
}


int dp83848_init_phy(int phy_addr)
{
	int	ret = 1;

	if (!dp83848_get_link_speed(phy_addr)) {
		/* Try another time */
		udelay(100000);
		ret = dp83848_get_link_speed(phy_addr);
	}

	/* Disable PHY Interrupts */
	davinci_eth_phy_write(phy_addr, DP83848_PHY_INTR_CTRL_REG, 0);

	return(ret);
}


int dp83848_auto_negotiate(int phy_addr)
{
	u_int16_t	tmp;


	if (!davinci_eth_phy_read(phy_addr, DP83848_CTL_REG, &tmp))
		return(0);

	/* Restart Auto_negotiation  */
	tmp &= ~DP83848_AUTONEG;	/* remove autonegotiation enable */
	tmp |= DP83848_ISOLATE;		/* Electrically isolate PHY */
	davinci_eth_phy_write(phy_addr, DP83848_CTL_REG, tmp);

	/* Set the Auto_negotiation Advertisement Register
	 * MII advertising for Next page, 100BaseTxFD and HD,
	 * 10BaseTFD and HD, IEEE 802.3
	 */
	tmp = DP83848_NP | DP83848_TX_FDX | DP83848_TX_HDX |
		DP83848_10_FDX | DP83848_10_HDX | DP83848_AN_IEEE_802_3;
	davinci_eth_phy_write(phy_addr, DP83848_ANA_REG, tmp);


	/* Read Control Register */
	if (!davinci_eth_phy_read(phy_addr, DP83848_CTL_REG, &tmp))
		return(0);

	tmp |= DP83848_SPEED_SELECT | DP83848_AUTONEG | DP83848_DUPLEX_MODE;
	davinci_eth_phy_write(phy_addr, DP83848_CTL_REG, tmp);

	/* Restart Auto_negotiation  */
	tmp |= DP83848_RESTART_AUTONEG;
	davinci_eth_phy_write(phy_addr, DP83848_CTL_REG, tmp);

	/*check AutoNegotiate complete */
	udelay(10000);
	if (!davinci_eth_phy_read(phy_addr, DP83848_STAT_REG, &tmp))
		return(0);

	if (!(tmp & DP83848_AUTONEG_COMP))
		return(0);

	return (dp83848_get_link_speed(phy_addr));
}

#endif	/* CONFIG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */

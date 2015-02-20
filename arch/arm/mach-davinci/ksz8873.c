/*
 * Micrel KSZ8873 PHY Driver for TI DaVinci
 * (TMS320DM644x) based boards.
 *
 * Copyright (C) 2011 Heiko Schocher <hsdenx.de>
 *
 * based on:
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
#include <miiphy.h>
#include <net.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include "../../../drivers/net/davinci_emac.h"

int ksz8873_is_phy_connected(int phy_addr)
{
	u_int16_t	dummy;

	return davinci_eth_phy_read(phy_addr, MII_PHYSID1, &dummy);
}

int ksz8873_get_link_speed(int phy_addr)
{
	emac_regs *emac = (emac_regs *)EMAC_BASE_ADDR;

	/* we always have a link to the switch, 100 FD */
	writel((EMAC_MACCONTROL_MIIEN_ENABLE |
		EMAC_MACCONTROL_FULLDUPLEX_ENABLE),
	       &emac->MACCONTROL);
	return 1;
}


int ksz8873_init_phy(int phy_addr)
{
	return 1;
}


int ksz8873_auto_negotiate(int phy_addr)
{
	return dp83848_get_link_speed(phy_addr);
}

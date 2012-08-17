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
#include <miiphy.h>
#include <net.h>
#include <asm/arch/emac_defs.h>
#include <asm/io.h>
#include "../../../../../drivers/net/davinci_emac.h"

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

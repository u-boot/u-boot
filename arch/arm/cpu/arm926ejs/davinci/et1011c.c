/*
 * LSI ET1011C PHY Driver for TI DaVinci(TMS320DM6467) board.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <common.h>
#include <net.h>
#include <miiphy.h>
#include <asm/arch/emac_defs.h>
#include "../../../../../drivers/net/davinci_emac.h"

#ifdef CONFIG_DRIVER_TI_EMAC

#ifdef CONFIG_CMD_NET

/* LSI PHYSICAL LAYER TRANSCEIVER ET1011C */

#define MII_PHY_CONFIG_REG		22

/* PHY Config bits */
#define PHY_SYS_CLK_EN			(1 << 4)

int et1011c_get_link_speed(int phy_addr)
{
	u_int16_t	data;

	if (davinci_eth_phy_read(phy_addr, MII_STATUS_REG, &data) && (data & 0x04)) {
		davinci_eth_phy_read(phy_addr, MII_PHY_CONFIG_REG, &data);
		/* Enable 125MHz clock sourced from PHY */
		davinci_eth_phy_write(phy_addr, MII_PHY_CONFIG_REG,
			data | PHY_SYS_CLK_EN);
		return (1);
	}
	return (0);
}

#endif	/* CONFIG_CMD_NET */

#endif	/* CONFIG_DRIVER_ETHER */

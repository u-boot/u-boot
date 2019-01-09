// SPDX-License-Identifier: GPL-2.0+
/*
 * LSI ET1011C PHY Driver for TI DaVinci(TMS320DM6467) board.
 *
 * Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 */

#include <common.h>
#include <net.h>
#include <miiphy.h>
#include <asm/arch/emac_defs.h>
#include "../../../drivers/net/ti/davinci_emac.h"

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

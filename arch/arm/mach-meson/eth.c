// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/gx.h>
#include <asm/arch/eth.h>
#include <phy.h>

/* Configure the Ethernet MAC with the requested interface mode
 * with some optional flags.
 */
void meson_gx_eth_init(phy_interface_t mode, unsigned int flags)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		/* Set RGMII mode */
		setbits_le32(GX_ETH_REG_0, GX_ETH_REG_0_PHY_INTF |
			     GX_ETH_REG_0_TX_PHASE(1) |
			     GX_ETH_REG_0_TX_RATIO(4) |
			     GX_ETH_REG_0_PHY_CLK_EN |
			     GX_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(GX_ETH_REG_0, GX_ETH_REG_0_INVERT_RMII_CLK |
					 GX_ETH_REG_0_CLK_EN);

		/* Use GXL RMII Internal PHY */
		if (IS_ENABLED(CONFIG_MESON_GXL) &&
		    (flags & MESON_GXL_USE_INTERNAL_RMII_PHY)) {
			writel(0x10110181, GX_ETH_REG_2);
			writel(0xe40908ff, GX_ETH_REG_3);
		}

		break;

	default:
		printf("Invalid Ethernet interface mode\n");
		return;
	}

	/* Enable power gate */
	clrbits_le32(GX_MEM_PD_REG_0, GX_MEM_PD_REG_0_ETH_MASK);
}

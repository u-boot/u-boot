/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/gxbb.h>
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
		setbits_le32(GXBB_ETH_REG_0, GXBB_ETH_REG_0_PHY_INTF |
			     GXBB_ETH_REG_0_TX_PHASE(1) |
			     GXBB_ETH_REG_0_TX_RATIO(4) |
			     GXBB_ETH_REG_0_PHY_CLK_EN |
			     GXBB_ETH_REG_0_CLK_EN);
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* Set RMII mode */
		out_le32(GXBB_ETH_REG_0, GXBB_ETH_REG_0_INVERT_RMII_CLK |
					 GXBB_ETH_REG_0_CLK_EN);

		/* Use GXL RMII Internal PHY */
		if (IS_ENABLED(CONFIG_MESON_GXL) &&
		    (flags & MESON_GXL_USE_INTERNAL_RMII_PHY)) {
			writel(0x10110181, GXBB_ETH_REG_2);
			writel(0xe40908ff, GXBB_ETH_REG_3);
		}

		break;

	default:
		printf("Invalid Ethernet interface mode\n");
		return;
	}

	/* Enable power and clock gate */
	setbits_le32(GXBB_GCLK_MPEG_1, GXBB_GCLK_MPEG_1_ETH);
	clrbits_le32(GXBB_MEM_PD_REG_0, GXBB_MEM_PD_REG_0_ETH_MASK);
}

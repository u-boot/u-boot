/*
 * (C) Copyright 2016 Beniamino Galvani <b.galvani@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/gxbb.h>
#include <dm/platdata.h>
#include <phy.h>

int board_init(void)
{
	return 0;
}

static const struct eth_pdata gxbb_eth_pdata = {
	.iobase = GXBB_ETH_BASE,
	.phy_interface = PHY_INTERFACE_MODE_RGMII,
};

U_BOOT_DEVICE(meson_eth) = {
	.name = "eth_designware",
	.platdata = &gxbb_eth_pdata,
};

int misc_init_r(void)
{
	/* Select Ethernet function */
	setbits_le32(GXBB_PINMUX(6), 0x3fff);

	/* Set RGMII mode */
	setbits_le32(GXBB_ETH_REG_0, GXBB_ETH_REG_0_PHY_INTF |
				     GXBB_ETH_REG_0_TX_PHASE(1) |
				     GXBB_ETH_REG_0_TX_RATIO(4) |
				     GXBB_ETH_REG_0_PHY_CLK_EN |
				     GXBB_ETH_REG_0_CLK_EN);

	/* Enable power and clock gate */
	setbits_le32(GXBB_GCLK_MPEG_1, GXBB_GCLK_MPEG_1_ETH);
	clrbits_le32(GXBB_MEM_PD_REG_0, GXBB_MEM_PD_REG_0_ETH_MASK);

	/* Reset PHY on GPIOZ_14 */
	clrbits_le32(GXBB_GPIO_EN(3), BIT(14));
	clrbits_le32(GXBB_GPIO_OUT(3), BIT(14));
	mdelay(10);
	setbits_le32(GXBB_GPIO_OUT(3), BIT(14));

	return 0;
}

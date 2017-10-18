/*
 * Meson GXL Internal PHY Driver
 *
 * Copyright (C) 2015 Amlogic, Inc. All rights reserved.
 * Copyright (C) 2016 BayLibre, SAS. All rights reserved.
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>
#include <common.h>
#include <linux/bitops.h>
#include <phy.h>

static int meson_gxl_phy_config(struct phy_device *phydev)
{
	/* Enable Analog and DSP register Bank access by */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0000);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0400);

	/* Write Analog register 23 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x8E0D);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x4417);

	/* Enable fractional PLL */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x0005);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x5C1B);

	/* Program fraction FR_PLL_DIV1 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0x029A);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x5C1D);

	/* Program fraction FR_PLL_DIV1 */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x17, 0xAAAA);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x5C1C);

	return genphy_config(phydev);
}

static struct phy_driver meson_gxl_phy_driver = {
	.name = "Meson GXL Internal PHY",
	.uid = 0x01814400,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &meson_gxl_phy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

int phy_meson_gxl_init(void)
{
	phy_register(&meson_gxl_phy_driver);

	return 0;
}

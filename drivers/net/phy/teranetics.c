/*
 * Teranetics PHY drivers
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
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 *
 */
#include <config.h>
#include <phy.h>

#ifndef CONFIG_PHYLIB_10G
#error The Teranetics PHY needs 10G support
#endif

int tn2020_config(struct phy_device *phydev)
{
	if (phydev->port == PORT_FIBRE) {
		unsigned short restart_an = (MDIO_AN_CTRL1_RESTART |
						MDIO_AN_CTRL1_ENABLE |
						MDIO_AN_CTRL1_XNP);

		phy_write(phydev, 30, 93, 2);
		phy_write(phydev, MDIO_MMD_AN, MDIO_CTRL1, restart_an);
	}

	return 0;
}

struct phy_driver tn2020_driver = {
	.name = "Teranetics TN2020",
	.uid = 0x00a19410,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_DEVS_PMAPMD | MDIO_DEVS_PCS |
			MDIO_DEVS_PHYXS | MDIO_DEVS_AN |
			MDIO_DEVS_VEND1 | MDIO_DEVS_VEND2),
	.config = &tn2020_config,
	.startup = &gen10g_startup,
	.shutdown = &gen10g_shutdown,
};

int phy_teranetics_init(void)
{
	phy_register(&tn2020_driver);

	return 0;
}

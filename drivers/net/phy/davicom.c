/*
 * Davicom PHY drivers
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
#include <phy.h>

#define MIIM_DM9161_SCR                0x10
#define MIIM_DM9161_SCR_INIT   0x0610

/* DM9161 Specified Configuration and Status Register */
#define MIIM_DM9161_SCSR       0x11
#define MIIM_DM9161_SCSR_100F  0x8000
#define MIIM_DM9161_SCSR_100H  0x4000
#define MIIM_DM9161_SCSR_10F   0x2000
#define MIIM_DM9161_SCSR_10H   0x1000

/* DM9161 10BT Configuration/Status */
#define MIIM_DM9161_10BTCSR    0x12
#define MIIM_DM9161_10BTCSR_INIT       0x7800


/* Davicom DM9161E */
static int dm9161_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_ISOLATE);
	/* Do not bypass the scrambler/descrambler */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_DM9161_SCR,
			MIIM_DM9161_SCR_INIT);
	/* Clear 10BTCSR to default */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_DM9161_10BTCSR,
			MIIM_DM9161_10BTCSR_INIT);

	genphy_config_aneg(phydev);

	return 0;
}

static int dm9161_parse_status(struct phy_device *phydev)
{
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_DM9161_SCSR);

	if (mii_reg & (MIIM_DM9161_SCSR_100F | MIIM_DM9161_SCSR_100H))
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	if (mii_reg & (MIIM_DM9161_SCSR_100F | MIIM_DM9161_SCSR_10F))
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	return 0;
}

static int dm9161_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	dm9161_parse_status(phydev);

	return 0;
}

static struct phy_driver DM9161_driver = {
	.name = "Davicom DM9161E",
	.uid = 0x181b880,
	.mask = 0xffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &dm9161_config,
	.startup = &dm9161_startup,
	.shutdown = &genphy_shutdown,
};

int phy_davicom_init(void)
{
	phy_register(&DM9161_driver);

	return 0;
}

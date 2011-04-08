/*
 * National Semiconductor PHY drivers
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

/* DP83865 Link and Auto-Neg Status Register */
#define MIIM_DP83865_LANR      0x11
#define MIIM_DP83865_SPD_MASK  0x0018
#define MIIM_DP83865_SPD_1000  0x0010
#define MIIM_DP83865_SPD_100   0x0008
#define MIIM_DP83865_DPX_FULL  0x0002


/* NatSemi DP83865 */
static int dp83865_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);
	genphy_config_aneg(phydev);

	return 0;
}

static int dp83865_parse_status(struct phy_device *phydev)
{
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_DP83865_LANR);

	switch (mii_reg & MIIM_DP83865_SPD_MASK) {

	case MIIM_DP83865_SPD_1000:
		phydev->speed = SPEED_1000;
		break;

	case MIIM_DP83865_SPD_100:
		phydev->speed = SPEED_100;
		break;

	default:
		phydev->speed = SPEED_10;
		break;

	}

	if (mii_reg & MIIM_DP83865_DPX_FULL)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	return 0;
}

static int dp83865_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	dp83865_parse_status(phydev);

	return 0;
}


static struct phy_driver DP83865_driver = {
	.name = "NatSemi DP83865",
	.uid = 0x20005c70,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &dp83865_config,
	.startup = &dp83865_startup,
	.shutdown = &genphy_shutdown,
};

int phy_natsemi_init(void)
{
	phy_register(&DP83865_driver);

	return 0;
}

/*
 * LXT PHY drivers
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

/* LXT971 Status 2 registers */
#define MIIM_LXT971_SR2                     0x11  /* Status Register 2  */
#define MIIM_LXT971_SR2_SPEED_MASK 0x4200
#define MIIM_LXT971_SR2_10HDX     0x0000  /*  10 Mbit half duplex selected */
#define MIIM_LXT971_SR2_10FDX     0x0200  /*  10 Mbit full duplex selected */
#define MIIM_LXT971_SR2_100HDX    0x4000  /* 100 Mbit half duplex selected */
#define MIIM_LXT971_SR2_100FDX    0x4200  /* 100 Mbit full duplex selected */


/* LXT971 */
static int lxt971_parse_status(struct phy_device *phydev)
{
	int mii_reg;
	int speed;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_LXT971_SR2);
	speed = mii_reg & MIIM_LXT971_SR2_SPEED_MASK;

	switch (speed) {
	case MIIM_LXT971_SR2_10HDX:
		phydev->speed = SPEED_10;
		phydev->duplex = DUPLEX_HALF;
		break;
	case MIIM_LXT971_SR2_10FDX:
		phydev->speed = SPEED_10;
		phydev->duplex = DUPLEX_FULL;
		break;
	case MIIM_LXT971_SR2_100HDX:
		phydev->speed = SPEED_100;
		phydev->duplex = DUPLEX_HALF;
		break;
	default:
		phydev->speed = SPEED_100;
		phydev->duplex = DUPLEX_FULL;
	}

	return 0;
}

static int lxt971_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	lxt971_parse_status(phydev);

	return 0;
}

static struct phy_driver LXT971_driver = {
	.name = "LXT971",
	.uid = 0x1378e0,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &lxt971_startup,
	.shutdown = &genphy_shutdown,
};

int phy_lxt_init(void)
{
	phy_register(&LXT971_driver);

	return 0;
}

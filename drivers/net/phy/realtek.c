/*
 * RealTek PHY drivers
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
#include <common.h>
#include <phy.h>

#define PHY_AUTONEGOTIATE_TIMEOUT 5000

/* RTL8211B PHY Status Register */
#define MIIM_RTL8211B_PHY_STATUS       0x11
#define MIIM_RTL8211B_PHYSTAT_SPEED    0xc000
#define MIIM_RTL8211B_PHYSTAT_GBIT     0x8000
#define MIIM_RTL8211B_PHYSTAT_100      0x4000
#define MIIM_RTL8211B_PHYSTAT_DUPLEX   0x2000
#define MIIM_RTL8211B_PHYSTAT_SPDDONE  0x0800
#define MIIM_RTL8211B_PHYSTAT_LINK     0x0400


/* RealTek RTL8211B */
static int rtl8211b_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	genphy_config_aneg(phydev);

	return 0;
}

static int rtl8211b_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_RTL8211B_PHY_STATUS);

	if (!(mii_reg & MIIM_RTL8211B_PHYSTAT_SPDDONE)) {
		int i = 0;

		/* in case of timeout ->link is cleared */
		phydev->link = 1;
		puts("Waiting for PHY realtime link");
		while (!(mii_reg & MIIM_RTL8211B_PHYSTAT_SPDDONE)) {
			/* Timeout reached ? */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				phydev->link = 0;
				break;
			}

			if ((i++ % 1000) == 0)
				putc('.');
			udelay(1000);	/* 1 ms */
			mii_reg = phy_read(phydev, MDIO_DEVAD_NONE,
					MIIM_RTL8211B_PHY_STATUS);
		}
		puts(" done\n");
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_RTL8211B_PHYSTAT_LINK)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	if (mii_reg & MIIM_RTL8211B_PHYSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = (mii_reg & MIIM_RTL8211B_PHYSTAT_SPEED);

	switch (speed) {
	case MIIM_RTL8211B_PHYSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_RTL8211B_PHYSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
	}

	return 0;
}

static int rtl8211b_startup(struct phy_device *phydev)
{
	/* Read the Status (2x to make sure link is right) */
	genphy_update_link(phydev);
	rtl8211b_parse_status(phydev);

	return 0;
}

static struct phy_driver RTL8211B_driver = {
	.name = "RealTek RTL8211B",
	.uid = 0x1cc910,
	.mask = 0xfffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &rtl8211b_config,
	.startup = &rtl8211b_startup,
	.shutdown = &genphy_shutdown,
};

int phy_realtek_init(void)
{
	phy_register(&RTL8211B_driver);

	return 0;
}

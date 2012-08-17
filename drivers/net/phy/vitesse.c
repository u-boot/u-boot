/*
 * Vitesse PHY drivers
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
#include <miiphy.h>

/* Cicada Auxiliary Control/Status Register */
#define MIIM_CIS82xx_AUX_CONSTAT	0x1c
#define MIIM_CIS82xx_AUXCONSTAT_INIT	0x0004
#define MIIM_CIS82xx_AUXCONSTAT_DUPLEX	0x0020
#define MIIM_CIS82xx_AUXCONSTAT_SPEED	0x0018
#define MIIM_CIS82xx_AUXCONSTAT_GBIT	0x0010
#define MIIM_CIS82xx_AUXCONSTAT_100	0x0008

/* Cicada Extended Control Register 1 */
#define MIIM_CIS82xx_EXT_CON1		0x17
#define MIIM_CIS8201_EXTCON1_INIT	0x0000

/* Cicada 8204 Extended PHY Control Register 1 */
#define MIIM_CIS8204_EPHY_CON		0x17
#define MIIM_CIS8204_EPHYCON_INIT	0x0006
#define MIIM_CIS8204_EPHYCON_RGMII	0x1100

/* Cicada 8204 Serial LED Control Register */
#define MIIM_CIS8204_SLED_CON		0x1b
#define MIIM_CIS8204_SLEDCON_INIT	0x1115

/* Vitesse VSC8601 Extended PHY Control Register 1 */
#define MIIM_VSC8601_EPHY_CON		0x17
#define MIIM_VSC8601_EPHY_CON_INIT_SKEW	0x1120
#define MIIM_VSC8601_SKEW_CTRL		0x1c

#define PHY_EXT_PAGE_ACCESS    0x1f

/* CIS8201 */
static int vitesse_config(struct phy_device *phydev)
{
	/* Override PHY config settings */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_AUX_CONSTAT,
			MIIM_CIS82xx_AUXCONSTAT_INIT);
	/* Set up the interface mode */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_EXT_CON1,
			MIIM_CIS8201_EXTCON1_INIT);

	genphy_config_aneg(phydev);

	return 0;
}

static int vitesse_parse_status(struct phy_device *phydev)
{
	int speed;
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_AUX_CONSTAT);

	if (mii_reg & MIIM_CIS82xx_AUXCONSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_CIS82xx_AUXCONSTAT_SPEED;
	switch (speed) {
	case MIIM_CIS82xx_AUXCONSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_CIS82xx_AUXCONSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int vitesse_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	vitesse_parse_status(phydev);

	return 0;
}

static int cis8204_config(struct phy_device *phydev)
{
	/* Override PHY config settings */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS82xx_AUX_CONSTAT,
			MIIM_CIS82xx_AUXCONSTAT_INIT);

	genphy_config_aneg(phydev);

	if ((phydev->interface == PHY_INTERFACE_MODE_RGMII) ||
			(phydev->interface == PHY_INTERFACE_MODE_RGMII) ||
			(phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID) ||
			(phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID))
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS8204_EPHY_CON,
				MIIM_CIS8204_EPHYCON_INIT |
				MIIM_CIS8204_EPHYCON_RGMII);
	else
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_CIS8204_EPHY_CON,
				MIIM_CIS8204_EPHYCON_INIT);

	return 0;
}

/* Vitesse VSC8601 */
int vsc8601_config(struct phy_device *phydev)
{
	/* Configure some basic stuff */
#ifdef CONFIG_SYS_VSC8601_SKEWFIX
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8601_EPHY_CON,
			MIIM_VSC8601_EPHY_CON_INIT_SKEW);
#if defined(CONFIG_SYS_VSC8601_SKEW_TX) && defined(CONFIG_SYS_VSC8601_SKEW_RX)
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 1);
#define VSC8101_SKEW \
	((CONFIG_SYS_VSC8601_SKEW_TX << 14) \
	| (CONFIG_SYS_VSC8601_SKEW_RX << 12))
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_VSC8601_SKEW_CTRL,
			VSC8101_SKEW);
	phy_write(phydev, MDIO_DEVAD_NONE, PHY_EXT_PAGE_ACCESS, 0);
#endif
#endif

	genphy_config_aneg(phydev);

	return 0;
}

static struct phy_driver VSC8211_driver = {
	.name	= "Vitesse VSC8211",
	.uid	= 0xfc4b0,
	.mask	= 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vitesse_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8221_driver = {
	.name = "Vitesse VSC8221",
	.uid = 0xfc550,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8244_driver = {
	.name = "Vitesse VSC8244",
	.uid = 0xfc6c0,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8234_driver = {
	.name = "Vitesse VSC8234",
	.uid = 0xfc620,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8601_driver = {
	.name = "Vitesse VSC8601",
	.uid = 0x70420,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vsc8601_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver VSC8641_driver = {
	.name = "Vitesse VSC8641",
	.uid = 0x70430,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

/* Vitesse bought Cicada, so we'll put these here */
static struct phy_driver cis8201_driver = {
	.name = "CIS8201",
	.uid = 0xfc410,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &vitesse_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver cis8204_driver = {
	.name = "Cicada Cis8204",
	.uid = 0xfc440,
	.mask = 0xffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &cis8204_config,
	.startup = &vitesse_startup,
	.shutdown = &genphy_shutdown,
};

int phy_vitesse_init(void)
{
	phy_register(&VSC8641_driver);
	phy_register(&VSC8601_driver);
	phy_register(&VSC8234_driver);
	phy_register(&VSC8244_driver);
	phy_register(&VSC8211_driver);
	phy_register(&VSC8221_driver);
	phy_register(&cis8201_driver);
	phy_register(&cis8204_driver);

	return 0;
}

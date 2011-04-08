/*
 * Marvell PHY drivers
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

/* 88E1011 PHY Status Register */
#define MIIM_88E1xxx_PHY_STATUS		0x11
#define MIIM_88E1xxx_PHYSTAT_SPEED	0xc000
#define MIIM_88E1xxx_PHYSTAT_GBIT	0x8000
#define MIIM_88E1xxx_PHYSTAT_100	0x4000
#define MIIM_88E1xxx_PHYSTAT_DUPLEX	0x2000
#define MIIM_88E1xxx_PHYSTAT_SPDDONE	0x0800
#define MIIM_88E1xxx_PHYSTAT_LINK	0x0400

#define MIIM_88E1xxx_PHY_SCR		0x10
#define MIIM_88E1xxx_PHY_MDI_X_AUTO	0x0060

/* 88E1111 PHY LED Control Register */
#define MIIM_88E1111_PHY_LED_CONTROL	24
#define MIIM_88E1111_PHY_LED_DIRECT	0x4100
#define MIIM_88E1111_PHY_LED_COMBINE	0x411C

/* 88E1118 PHY defines */
#define MIIM_88E1118_PHY_PAGE		22
#define MIIM_88E1118_PHY_LED_PAGE	3

/* 88E1121 PHY LED Control Register */
#define MIIM_88E1121_PHY_LED_CTRL	16
#define MIIM_88E1121_PHY_LED_PAGE	3
#define MIIM_88E1121_PHY_LED_DEF	0x0030

/* 88E1121 PHY IRQ Enable/Status Register */
#define MIIM_88E1121_PHY_IRQ_EN		18
#define MIIM_88E1121_PHY_IRQ_STATUS	19

#define MIIM_88E1121_PHY_PAGE		22

/* 88E1145 Extended PHY Specific Control Register */
#define MIIM_88E1145_PHY_EXT_CR 20
#define MIIM_M88E1145_RGMII_RX_DELAY	0x0080
#define MIIM_M88E1145_RGMII_TX_DELAY	0x0002

#define MIIM_88E1145_PHY_LED_CONTROL	24
#define MIIM_88E1145_PHY_LED_DIRECT	0x4100

#define MIIM_88E1145_PHY_PAGE	29
#define MIIM_88E1145_PHY_CAL_OV 30

#define MIIM_88E1149_PHY_PAGE	29

/* Marvell 88E1011S */
static int m88e1011s_config(struct phy_device *phydev)
{
	/* Reset and configure the PHY */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x200c);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1d, 0x5);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	genphy_config_aneg(phydev);

	return 0;
}

/* Parse the 88E1011's status register for speed and duplex
 * information
 */
static uint m88e1xxx_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1xxx_PHY_STATUS);

	if ((mii_reg & MIIM_88E1xxx_PHYSTAT_LINK) &&
		!(mii_reg & MIIM_88E1xxx_PHYSTAT_SPDDONE)) {
		int i = 0;

		puts("Waiting for PHY realtime link");
		while (!(mii_reg & MIIM_88E1xxx_PHYSTAT_SPDDONE)) {
			/* Timeout reached ? */
			if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
				puts(" TIMEOUT !\n");
				phydev->link = 0;
				break;
			}

			if ((i++ % 1000) == 0)
				putc('.');
			udelay(1000);
			mii_reg = phy_read(phydev, MDIO_DEVAD_NONE,
					MIIM_88E1xxx_PHY_STATUS);
		}
		puts(" done\n");
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_88E1xxx_PHYSTAT_LINK)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	if (mii_reg & MIIM_88E1xxx_PHYSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = mii_reg & MIIM_88E1xxx_PHYSTAT_SPEED;

	switch (speed) {
	case MIIM_88E1xxx_PHYSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_88E1xxx_PHYSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int m88e1011s_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	m88e1xxx_parse_status(phydev);

	return 0;
}

/* Marvell 88E1111S */
static int m88e1111s_config(struct phy_device *phydev)
{
	int reg;

	if ((phydev->interface == PHY_INTERFACE_MODE_RGMII) ||
			(phydev->interface == PHY_INTERFACE_MODE_RGMII_ID) ||
			(phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID) ||
			(phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID)) {
		reg = phy_read(phydev, MDIO_DEVAD_NONE, 0x1b);
		reg = (reg & 0xfff0) | 0xb;
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1b, reg);
	} else {
		phy_write(phydev, MDIO_DEVAD_NONE, 0x1b, 0x1f);
	}

	phy_write(phydev, MDIO_DEVAD_NONE, 0x14, 0x0cd2);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

/* Marvell 88E1118 */
static int m88e1118_config(struct phy_device *phydev)
{
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0002);
	/* Delay RGMII TX and RX */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x15, 0x1070);
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0003);
	/* Adjust LED control */
	phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x021e);
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

static int m88e1118_startup(struct phy_device *phydev)
{
	/* Change Page Number */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1118_PHY_PAGE, 0x0000);

	genphy_update_link(phydev);
	m88e1xxx_parse_status(phydev);

	return 0;
}

/* Marvell 88E1121R */
static int m88e1121_config(struct phy_device *phydev)
{
	int pg;

	/* Configure the PHY */
	genphy_config_aneg(phydev);

	/* Switch the page to access the led register */
	pg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE,
			MIIM_88E1121_PHY_LED_PAGE);
	/* Configure leds */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_LED_CTRL,
			MIIM_88E1121_PHY_LED_DEF);
	/* Restore the page pointer */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_PAGE, pg);

	/* Disable IRQs and de-assert interrupt */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_IRQ_EN, 0);
	phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1121_PHY_IRQ_STATUS);

	return 0;
}

/* Marvell 88E1145 */
static int m88e1145_config(struct phy_device *phydev)
{
	int reg;

	/* Errata E0, E1 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_PAGE, 0x001b);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_CAL_OV, 0x418f);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_PAGE, 0x0016);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_CAL_OV, 0xa2da);

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1xxx_PHY_SCR,
			MIIM_88E1xxx_PHY_MDI_X_AUTO);

	reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_EXT_CR);
	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		reg |= MIIM_M88E1145_RGMII_RX_DELAY |
			MIIM_M88E1145_RGMII_TX_DELAY;
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_EXT_CR, reg);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

static int m88e1145_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1145_PHY_LED_CONTROL,
			MIIM_88E1145_PHY_LED_DIRECT);
	m88e1xxx_parse_status(phydev);

	return 0;
}

/* Marvell 88E1149S */
static int m88e1149_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1149_PHY_PAGE, 0x1f);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x200c);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_88E1149_PHY_PAGE, 0x5);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x0);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x1e, 0x100);

	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}


static struct phy_driver M88E1011S_driver = {
	.name = "Marvell 88E1011S",
	.uid = 0x1410c60,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1011s_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1111S_driver = {
	.name = "Marvell 88E1111S",
	.uid = 0x1410cc0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1111s_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1118_driver = {
	.name = "Marvell 88E1118",
	.uid = 0x1410e10,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1118_config,
	.startup = &m88e1118_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1121R_driver = {
	.name = "Marvell 88E1121R",
	.uid = 0x1410cb0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1121_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1145_driver = {
	.name = "Marvell 88E1145",
	.uid = 0x1410cd0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1145_config,
	.startup = &m88e1145_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver M88E1149S_driver = {
	.name = "Marvell 88E1149S",
	.uid = 0x1410ca0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &m88e1149_config,
	.startup = &m88e1011s_startup,
	.shutdown = &genphy_shutdown,
};

int phy_marvell_init(void)
{
	phy_register(&M88E1149S_driver);
	phy_register(&M88E1145_driver);
	phy_register(&M88E1121R_driver);
	phy_register(&M88E1118_driver);
	phy_register(&M88E1111S_driver);
	phy_register(&M88E1011S_driver);

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Aquantia PHY drivers
 *
 * Copyright 2014 Freescale Semiconductor, Inc.
 */
#include <config.h>
#include <common.h>
#include <dm.h>
#include <phy.h>

#ifndef CONFIG_PHYLIB_10G
#error The Aquantia PHY needs 10G support
#endif

#define AQUNTIA_10G_CTL		0x20
#define AQUNTIA_VENDOR_P1	0xc400

#define AQUNTIA_SPEED_LSB_MASK	0x2000
#define AQUNTIA_SPEED_MSB_MASK	0x40

int aquantia_config(struct phy_device *phydev)
{
	u32 val = phy_read(phydev, MDIO_MMD_PMAPMD, MII_BMCR);

	if (phydev->interface == PHY_INTERFACE_MODE_SGMII) {
		/* 1000BASE-T mode */
		phydev->advertising = SUPPORTED_1000baseT_Full;
		phydev->supported = phydev->advertising;

		val = (val & ~AQUNTIA_SPEED_LSB_MASK) | AQUNTIA_SPEED_MSB_MASK;
		phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR, val);
	} else if (phydev->interface == PHY_INTERFACE_MODE_XGMII) {
		/* 10GBASE-T mode */
		phydev->advertising = SUPPORTED_10000baseT_Full;
		phydev->supported = phydev->advertising;

		if (!(val & AQUNTIA_SPEED_LSB_MASK) ||
		    !(val & AQUNTIA_SPEED_MSB_MASK))
			phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR,
				  AQUNTIA_SPEED_LSB_MASK |
				  AQUNTIA_SPEED_MSB_MASK);
	} else if (phydev->interface == PHY_INTERFACE_MODE_SGMII_2500) {
		/* 2.5GBASE-T mode */
		phydev->advertising = SUPPORTED_1000baseT_Full;
		phydev->supported = phydev->advertising;

		phy_write(phydev, MDIO_MMD_AN, AQUNTIA_10G_CTL, 1);
		phy_write(phydev, MDIO_MMD_AN, AQUNTIA_VENDOR_P1, 0x9440);
	} else if (phydev->interface == PHY_INTERFACE_MODE_MII) {
		/* 100BASE-TX mode */
		phydev->advertising = SUPPORTED_100baseT_Full;
		phydev->supported = phydev->advertising;

		val = (val & ~AQUNTIA_SPEED_MSB_MASK) | AQUNTIA_SPEED_LSB_MASK;
		phy_write(phydev, MDIO_MMD_PMAPMD, MII_BMCR, val);
	}
	return 0;
}

int aquantia_startup(struct phy_device *phydev)
{
	u32 reg, speed;
	int i = 0;

	phydev->duplex = DUPLEX_FULL;

	/* if the AN is still in progress, wait till timeout. */
	phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	reg = phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	if (!(reg & MDIO_AN_STAT1_COMPLETE)) {
		printf("%s Waiting for PHY auto negotiation to complete",
		       phydev->dev->name);
		do {
			udelay(1000);
			reg = phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
			if ((i++ % 500) == 0)
				printf(".");
		} while (!(reg & MDIO_AN_STAT1_COMPLETE) &&
			 i < (4 * PHY_ANEG_TIMEOUT));

		if (i > PHY_ANEG_TIMEOUT)
			printf(" TIMEOUT !\n");
	}

	/* Read twice because link state is latched and a
	 * read moves the current state into the register */
	phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	reg = phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	if (reg < 0 || !(reg & MDIO_STAT1_LSTATUS))
		phydev->link = 0;
	else
		phydev->link = 1;

	speed = phy_read(phydev, MDIO_MMD_PMAPMD, MII_BMCR);
	if (speed & AQUNTIA_SPEED_MSB_MASK) {
		if (speed & AQUNTIA_SPEED_LSB_MASK)
			phydev->speed = SPEED_10000;
		else
			phydev->speed = SPEED_1000;
	} else {
		if (speed & AQUNTIA_SPEED_LSB_MASK)
			phydev->speed = SPEED_100;
		else
			phydev->speed = SPEED_10;
	}

	return 0;
}

struct phy_driver aq1202_driver = {
	.name = "Aquantia AQ1202",
	.uid = 0x3a1b445,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

struct phy_driver aq2104_driver = {
	.name = "Aquantia AQ2104",
	.uid = 0x3a1b460,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

struct phy_driver aqr105_driver = {
	.name = "Aquantia AQR105",
	.uid = 0x3a1b4a2,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

struct phy_driver aqr106_driver = {
	.name = "Aquantia AQR106",
	.uid = 0x3a1b4d0,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

struct phy_driver aqr107_driver = {
	.name = "Aquantia AQR107",
	.uid = 0x3a1b4e0,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
			MDIO_MMD_PHYXS | MDIO_MMD_AN |
			MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

struct phy_driver aqr405_driver = {
	.name = "Aquantia AQR405",
	.uid = 0x3a1b4b2,
	.mask = 0xfffffff0,
	.features = PHY_10G_FEATURES,
	.mmds = (MDIO_MMD_PMAPMD | MDIO_MMD_PCS|
		 MDIO_MMD_PHYXS | MDIO_MMD_AN |
		 MDIO_MMD_VEND1),
	.config = &aquantia_config,
	.startup = &aquantia_startup,
	.shutdown = &gen10g_shutdown,
};

int phy_aquantia_init(void)
{
	phy_register(&aq1202_driver);
	phy_register(&aq2104_driver);
	phy_register(&aqr105_driver);
	phy_register(&aqr106_driver);
	phy_register(&aqr107_driver);
	phy_register(&aqr405_driver);

	return 0;
}

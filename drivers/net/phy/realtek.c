/*
 * RealTek PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright 2010-2011, 2015 Freescale Semiconductor, Inc.
 * author Andy Fleming
 * Copyright 2016 Karsten Merker <merker@debian.org>
 */
#include <config.h>
#include <common.h>
#include <linux/bitops.h>
#include <phy.h>

#define PHY_RTL8211x_FORCE_MASTER BIT(1)

#define PHY_AUTONEGOTIATE_TIMEOUT 5000

/* RTL8211x 1000BASE-T Control Register */
#define MIIM_RTL8211x_CTRL1000T_MSCE BIT(12);
#define MIIM_RTL8211x_CTRL1000T_MASTER BIT(11);

/* RTL8211x PHY Status Register */
#define MIIM_RTL8211x_PHY_STATUS       0x11
#define MIIM_RTL8211x_PHYSTAT_SPEED    0xc000
#define MIIM_RTL8211x_PHYSTAT_GBIT     0x8000
#define MIIM_RTL8211x_PHYSTAT_100      0x4000
#define MIIM_RTL8211x_PHYSTAT_DUPLEX   0x2000
#define MIIM_RTL8211x_PHYSTAT_SPDDONE  0x0800
#define MIIM_RTL8211x_PHYSTAT_LINK     0x0400

/* RTL8211x PHY Interrupt Enable Register */
#define MIIM_RTL8211x_PHY_INER         0x12
#define MIIM_RTL8211x_PHY_INTR_ENA     0x9f01
#define MIIM_RTL8211x_PHY_INTR_DIS     0x0000

/* RTL8211x PHY Interrupt Status Register */
#define MIIM_RTL8211x_PHY_INSR         0x13

/* RTL8211F PHY Status Register */
#define MIIM_RTL8211F_PHY_STATUS       0x1a
#define MIIM_RTL8211F_AUTONEG_ENABLE   0x1000
#define MIIM_RTL8211F_PHYSTAT_SPEED    0x0030
#define MIIM_RTL8211F_PHYSTAT_GBIT     0x0020
#define MIIM_RTL8211F_PHYSTAT_100      0x0010
#define MIIM_RTL8211F_PHYSTAT_DUPLEX   0x0008
#define MIIM_RTL8211F_PHYSTAT_SPDDONE  0x0800
#define MIIM_RTL8211F_PHYSTAT_LINK     0x0004

#define MIIM_RTL8211F_PAGE_SELECT      0x1f
#define MIIM_RTL8211F_TX_DELAY		0x100
#define MIIM_RTL8211F_LCR		0x10

static int rtl8211b_probe(struct phy_device *phydev)
{
#ifdef CONFIG_RTL8211X_PHY_FORCE_MASTER
	phydev->flags |= PHY_RTL8211x_FORCE_MASTER;
#endif

	return 0;
}

/* RealTek RTL8211x */
static int rtl8211x_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	/* mask interrupt at init; if the interrupt is
	 * needed indeed, it should be explicitly enabled
	 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_RTL8211x_PHY_INER,
		  MIIM_RTL8211x_PHY_INTR_DIS);

	if (phydev->flags & PHY_RTL8211x_FORCE_MASTER) {
		unsigned int reg;

		reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_CTRL1000);
		/* force manual master/slave configuration */
		reg |= MIIM_RTL8211x_CTRL1000T_MSCE;
		/* force master mode */
		reg |= MIIM_RTL8211x_CTRL1000T_MASTER;
		phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, reg);
	}
	/* read interrupt status just to clear it */
	phy_read(phydev, MDIO_DEVAD_NONE, MIIM_RTL8211x_PHY_INER);

	genphy_config_aneg(phydev);

	return 0;
}

static int rtl8211f_config(struct phy_device *phydev)
{
	u16 reg;

	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, BMCR_RESET);

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII) {
		/* enable TXDLY */
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_RTL8211F_PAGE_SELECT, 0xd08);
		reg = phy_read(phydev, MDIO_DEVAD_NONE, 0x11);
		reg |= MIIM_RTL8211F_TX_DELAY;
		phy_write(phydev, MDIO_DEVAD_NONE, 0x11, reg);
		/* restore to default page 0 */
		phy_write(phydev, MDIO_DEVAD_NONE,
			  MIIM_RTL8211F_PAGE_SELECT, 0x0);
	}

	/* Set green LED for Link, yellow LED for Active */
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MIIM_RTL8211F_PAGE_SELECT, 0xd04);
	phy_write(phydev, MDIO_DEVAD_NONE, 0x10, 0x617f);
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MIIM_RTL8211F_PAGE_SELECT, 0x0);

	genphy_config_aneg(phydev);

	return 0;
}

static int rtl8211x_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_RTL8211x_PHY_STATUS);

	if (!(mii_reg & MIIM_RTL8211x_PHYSTAT_SPDDONE)) {
		int i = 0;

		/* in case of timeout ->link is cleared */
		phydev->link = 1;
		puts("Waiting for PHY realtime link");
		while (!(mii_reg & MIIM_RTL8211x_PHYSTAT_SPDDONE)) {
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
					MIIM_RTL8211x_PHY_STATUS);
		}
		puts(" done\n");
		udelay(500000);	/* another 500 ms (results in faster booting) */
	} else {
		if (mii_reg & MIIM_RTL8211x_PHYSTAT_LINK)
			phydev->link = 1;
		else
			phydev->link = 0;
	}

	if (mii_reg & MIIM_RTL8211x_PHYSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = (mii_reg & MIIM_RTL8211x_PHYSTAT_SPEED);

	switch (speed) {
	case MIIM_RTL8211x_PHYSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_RTL8211x_PHYSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
	}

	return 0;
}

static int rtl8211f_parse_status(struct phy_device *phydev)
{
	unsigned int speed;
	unsigned int mii_reg;
	int i = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_RTL8211F_PAGE_SELECT, 0xa43);
	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_RTL8211F_PHY_STATUS);

	phydev->link = 1;
	while (!(mii_reg & MIIM_RTL8211F_PHYSTAT_LINK)) {
		if (i > PHY_AUTONEGOTIATE_TIMEOUT) {
			puts(" TIMEOUT !\n");
			phydev->link = 0;
			break;
		}

		if ((i++ % 1000) == 0)
			putc('.');
		udelay(1000);
		mii_reg = phy_read(phydev, MDIO_DEVAD_NONE,
				   MIIM_RTL8211F_PHY_STATUS);
	}

	if (mii_reg & MIIM_RTL8211F_PHYSTAT_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	speed = (mii_reg & MIIM_RTL8211F_PHYSTAT_SPEED);

	switch (speed) {
	case MIIM_RTL8211F_PHYSTAT_GBIT:
		phydev->speed = SPEED_1000;
		break;
	case MIIM_RTL8211F_PHYSTAT_100:
		phydev->speed = SPEED_100;
		break;
	default:
		phydev->speed = SPEED_10;
	}

	return 0;
}

static int rtl8211x_startup(struct phy_device *phydev)
{
	int ret;

	/* Read the Status (2x to make sure link is right) */
	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return rtl8211x_parse_status(phydev);
}

static int rtl8211e_startup(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	return genphy_parse_link(phydev);
}

static int rtl8211f_startup(struct phy_device *phydev)
{
	int ret;

	/* Read the Status (2x to make sure link is right) */
	ret = genphy_update_link(phydev);
	if (ret)
		return ret;
	/* Read the Status (2x to make sure link is right) */

	return rtl8211f_parse_status(phydev);
}

/* Support for RTL8211B PHY */
static struct phy_driver RTL8211B_driver = {
	.name = "RealTek RTL8211B",
	.uid = 0x1cc912,
	.mask = 0xffffff,
	.features = PHY_GBIT_FEATURES,
	.probe = &rtl8211b_probe,
	.config = &rtl8211x_config,
	.startup = &rtl8211x_startup,
	.shutdown = &genphy_shutdown,
};

/* Support for RTL8211E-VB-CG, RTL8211E-VL-CG and RTL8211EG-VB-CG PHYs */
static struct phy_driver RTL8211E_driver = {
	.name = "RealTek RTL8211E",
	.uid = 0x1cc915,
	.mask = 0xffffff,
	.features = PHY_GBIT_FEATURES,
	.config = &rtl8211x_config,
	.startup = &rtl8211e_startup,
	.shutdown = &genphy_shutdown,
};

/* Support for RTL8211DN PHY */
static struct phy_driver RTL8211DN_driver = {
	.name = "RealTek RTL8211DN",
	.uid = 0x1cc914,
	.mask = 0xffffff,
	.features = PHY_GBIT_FEATURES,
	.config = &rtl8211x_config,
	.startup = &rtl8211x_startup,
	.shutdown = &genphy_shutdown,
};

/* Support for RTL8211F PHY */
static struct phy_driver RTL8211F_driver = {
	.name = "RealTek RTL8211F",
	.uid = 0x1cc916,
	.mask = 0xffffff,
	.features = PHY_GBIT_FEATURES,
	.config = &rtl8211f_config,
	.startup = &rtl8211f_startup,
	.shutdown = &genphy_shutdown,
};

int phy_realtek_init(void)
{
	phy_register(&RTL8211B_driver);
	phy_register(&RTL8211E_driver);
	phy_register(&RTL8211F_driver);
	phy_register(&RTL8211DN_driver);

	return 0;
}

/*
 * Broadcom PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 */
#include <config.h>
#include <common.h>
#include <phy.h>

/* Broadcom BCM54xx -- taken from linux sungem_phy */
#define MIIM_BCM54xx_AUXCNTL			0x18
#define MIIM_BCM54xx_AUXCNTL_ENCODE(val) (((val & 0x7) << 12)|(val & 0x7))
#define MIIM_BCM54xx_AUXSTATUS			0x19
#define MIIM_BCM54xx_AUXSTATUS_LINKMODE_MASK	0x0700
#define MIIM_BCM54xx_AUXSTATUS_LINKMODE_SHIFT	8

#define MIIM_BCM54XX_SHD			0x1c
#define MIIM_BCM54XX_SHD_WRITE			0x8000
#define MIIM_BCM54XX_SHD_VAL(x)			((x & 0x1f) << 10)
#define MIIM_BCM54XX_SHD_DATA(x)		((x & 0x3ff) << 0)
#define MIIM_BCM54XX_SHD_WR_ENCODE(val, data)	\
	(MIIM_BCM54XX_SHD_WRITE | MIIM_BCM54XX_SHD_VAL(val) | \
	 MIIM_BCM54XX_SHD_DATA(data))

#define MIIM_BCM54XX_EXP_DATA		0x15	/* Expansion register data */
#define MIIM_BCM54XX_EXP_SEL		0x17	/* Expansion register select */
#define MIIM_BCM54XX_EXP_SEL_SSD	0x0e00	/* Secondary SerDes select */
#define MIIM_BCM54XX_EXP_SEL_ER		0x0f00	/* Expansion register select */

/* Broadcom BCM5461S */
static int bcm5461_config(struct phy_device *phydev)
{
	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

static int bcm54xx_parse_status(struct phy_device *phydev)
{
	unsigned int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_BCM54xx_AUXSTATUS);

	switch ((mii_reg & MIIM_BCM54xx_AUXSTATUS_LINKMODE_MASK) >>
			MIIM_BCM54xx_AUXSTATUS_LINKMODE_SHIFT) {
	case 1:
		phydev->duplex = DUPLEX_HALF;
		phydev->speed = SPEED_10;
		break;
	case 2:
		phydev->duplex = DUPLEX_FULL;
		phydev->speed = SPEED_10;
		break;
	case 3:
		phydev->duplex = DUPLEX_HALF;
		phydev->speed = SPEED_100;
		break;
	case 5:
		phydev->duplex = DUPLEX_FULL;
		phydev->speed = SPEED_100;
		break;
	case 6:
		phydev->duplex = DUPLEX_HALF;
		phydev->speed = SPEED_1000;
		break;
	case 7:
		phydev->duplex = DUPLEX_FULL;
		phydev->speed = SPEED_1000;
		break;
	default:
		printf("Auto-neg error, defaulting to 10BT/HD\n");
		phydev->duplex = DUPLEX_HALF;
		phydev->speed = SPEED_10;
		break;
	}

	return 0;
}

static int bcm54xx_startup(struct phy_device *phydev)
{
	/* Read the Status (2x to make sure link is right) */
	genphy_update_link(phydev);
	bcm54xx_parse_status(phydev);

	return 0;
}

/* Broadcom BCM5482S */
/*
 * "Ethernet@Wirespeed" needs to be enabled to achieve link in certain
 * circumstances.  eg a gigabit TSEC connected to a gigabit switch with
 * a 4-wire ethernet cable.  Both ends advertise gigabit, but can't
 * link.  "Ethernet@Wirespeed" reduces advertised speed until link
 * can be achieved.
 */
static u32 bcm5482_read_wirespeed(struct phy_device *phydev, u32 reg)
{
	return (phy_read(phydev, MDIO_DEVAD_NONE, reg) & 0x8FFF) | 0x8010;
}

static int bcm5482_config(struct phy_device *phydev)
{
	unsigned int reg;

	/* reset the PHY */
	reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMCR);
	reg |= BMCR_RESET;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR, reg);

	/* Setup read from auxilary control shadow register 7 */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54xx_AUXCNTL,
			MIIM_BCM54xx_AUXCNTL_ENCODE(7));
	/* Read Misc Control register and or in Ethernet@Wirespeed */
	reg = bcm5482_read_wirespeed(phydev, MIIM_BCM54xx_AUXCNTL);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54xx_AUXCNTL, reg);

	/* Initial config/enable of secondary SerDes interface */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_SHD,
			MIIM_BCM54XX_SHD_WR_ENCODE(0x14, 0xf));
	/* Write intial value to secondary SerDes Contol */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_EXP_SEL,
			MIIM_BCM54XX_EXP_SEL_SSD | 0);
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_EXP_DATA,
			BMCR_ANRESTART);
	/* Enable copper/fiber auto-detect */
	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_SHD,
			MIIM_BCM54XX_SHD_WR_ENCODE(0x1e, 0x201));

	genphy_config_aneg(phydev);

	return 0;
}

static int bcm_cygnus_startup(struct phy_device *phydev)
{
	/* Read the Status (2x to make sure link is right) */
	genphy_update_link(phydev);
	genphy_parse_link(phydev);

	return 0;
}

static int bcm_cygnus_config(struct phy_device *phydev)
{
	genphy_config_aneg(phydev);

	phy_reset(phydev);

	return 0;
}

/*
 * Find out if PHY is in copper or serdes mode by looking at Expansion Reg
 * 0x42 - "Operating Mode Status Register"
 */
static int bcm5482_is_serdes(struct phy_device *phydev)
{
	u16 val;
	int serdes = 0;

	phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_EXP_SEL,
			MIIM_BCM54XX_EXP_SEL_ER | 0x42);
	val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_EXP_DATA);

	switch (val & 0x1f) {
	case 0x0d:	/* RGMII-to-100Base-FX */
	case 0x0e:	/* RGMII-to-SGMII */
	case 0x0f:	/* RGMII-to-SerDes */
	case 0x12:	/* SGMII-to-SerDes */
	case 0x13:	/* SGMII-to-100Base-FX */
	case 0x16:	/* SerDes-to-Serdes */
		serdes = 1;
		break;
	case 0x6:	/* RGMII-to-Copper */
	case 0x14:	/* SGMII-to-Copper */
	case 0x17:	/* SerDes-to-Copper */
		break;
	default:
		printf("ERROR, invalid PHY mode (0x%x\n)", val);
		break;
	}

	return serdes;
}

/*
 * Determine SerDes link speed and duplex from Expansion reg 0x42 "Operating
 * Mode Status Register"
 */
static u32 bcm5482_parse_serdes_sr(struct phy_device *phydev)
{
	u16 val;
	int i = 0;

	/* Wait 1s for link - Clause 37 autonegotiation happens very fast */
	while (1) {
		phy_write(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_EXP_SEL,
				MIIM_BCM54XX_EXP_SEL_ER | 0x42);
		val = phy_read(phydev, MDIO_DEVAD_NONE, MIIM_BCM54XX_EXP_DATA);

		if (val & 0x8000)
			break;

		if (i++ > 1000) {
			phydev->link = 0;
			return 1;
		}

		udelay(1000);	/* 1 ms */
	}

	phydev->link = 1;
	switch ((val >> 13) & 0x3) {
	case (0x00):
		phydev->speed = 10;
		break;
	case (0x01):
		phydev->speed = 100;
		break;
	case (0x02):
		phydev->speed = 1000;
		break;
	}

	phydev->duplex = (val & 0x1000) == 0x1000;

	return 0;
}

/*
 * Figure out if BCM5482 is in serdes or copper mode and determine link
 * configuration accordingly
 */
static int bcm5482_startup(struct phy_device *phydev)
{
	if (bcm5482_is_serdes(phydev)) {
		bcm5482_parse_serdes_sr(phydev);
		phydev->port = PORT_FIBRE;
	} else {
		/* Wait for auto-negotiation to complete or fail */
		genphy_update_link(phydev);
		/* Parse BCM54xx copper aux status register */
		bcm54xx_parse_status(phydev);
	}

	return 0;
}

static struct phy_driver BCM5461S_driver = {
	.name = "Broadcom BCM5461S",
	.uid = 0x2060c0,
	.mask = 0xfffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &bcm5461_config,
	.startup = &bcm54xx_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver BCM5464S_driver = {
	.name = "Broadcom BCM5464S",
	.uid = 0x2060b0,
	.mask = 0xfffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &bcm5461_config,
	.startup = &bcm54xx_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver BCM5482S_driver = {
	.name = "Broadcom BCM5482S",
	.uid = 0x143bcb0,
	.mask = 0xffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &bcm5482_config,
	.startup = &bcm5482_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver BCM_CYGNUS_driver = {
	.name = "Broadcom CYGNUS GPHY",
	.uid = 0xae025200,
	.mask = 0xfffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &bcm_cygnus_config,
	.startup = &bcm_cygnus_startup,
	.shutdown = &genphy_shutdown,
};

int phy_broadcom_init(void)
{
	phy_register(&BCM5482S_driver);
	phy_register(&BCM5464S_driver);
	phy_register(&BCM5461S_driver);
	phy_register(&BCM_CYGNUS_driver);

	return 0;
}

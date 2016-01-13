/*
 * Micrel PHY drivers
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 * author Andy Fleming
 * (C) 2012 NetModule AG, David Andrey, added KSZ9031
 */
#include <config.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <micrel.h>
#include <phy.h>

DECLARE_GLOBAL_DATA_PTR;

static struct phy_driver KSZ804_driver = {
	.name = "Micrel KSZ804",
	.uid = 0x221510,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver KSZ8031_driver = {
	.name = "Micrel KSZ8021/KSZ8031",
	.uid = 0x221550,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

/**
 * KSZ8051
 */
#define MII_KSZ8051_PHY_OMSO			0x16
#define MII_KSZ8051_PHY_OMSO_NAND_TREE_ON	(1 << 5)

static int ksz8051_config(struct phy_device *phydev)
{
	unsigned val;

	/* Disable NAND-tree */
	val = phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZ8051_PHY_OMSO);
	val &= ~MII_KSZ8051_PHY_OMSO_NAND_TREE_ON;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_KSZ8051_PHY_OMSO, val);

	return genphy_config(phydev);
}

static struct phy_driver KSZ8051_driver = {
	.name = "Micrel KSZ8051",
	.uid = 0x221550,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &ksz8051_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver KSZ8081_driver = {
	.name = "Micrel KSZ8081",
	.uid = 0x221560,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

/**
 * KSZ8895
 */

static unsigned short smireg_to_phy(unsigned short reg)
{
	return ((reg & 0xc0) >> 3) + 0x06 + ((reg & 0x20) >> 5);
}

static unsigned short smireg_to_reg(unsigned short reg)
{
	return reg & 0x1F;
}

static void ksz8895_write_smireg(struct phy_device *phydev, int smireg, int val)
{
	phydev->bus->write(phydev->bus, smireg_to_phy(smireg), MDIO_DEVAD_NONE,
						smireg_to_reg(smireg), val);
}

#if 0
static int ksz8895_read_smireg(struct phy_device *phydev, int smireg)
{
	return phydev->bus->read(phydev->bus, smireg_to_phy(smireg),
					MDIO_DEVAD_NONE, smireg_to_reg(smireg));
}
#endif

int ksz8895_config(struct phy_device *phydev)
{
	/* we are connected directly to the switch without
	 * dedicated PHY. SCONF1 == 001 */
	phydev->link = 1;
	phydev->duplex = DUPLEX_FULL;
	phydev->speed = SPEED_100;

	/* Force the switch to start */
	ksz8895_write_smireg(phydev, 1, 1);

	return 0;
}

static int ksz8895_startup(struct phy_device *phydev)
{
	return 0;
}

static struct phy_driver ksz8895_driver = {
	.name = "Micrel KSZ8895/KSZ8864",
	.uid  = 0x221450,
	.mask = 0xffffe1,
	.features = PHY_BASIC_FEATURES,
	.config   = &ksz8895_config,
	.startup  = &ksz8895_startup,
	.shutdown = &genphy_shutdown,
};

#ifndef CONFIG_PHY_MICREL_KSZ9021
/*
 * I can't believe Micrel used the exact same part number
 * for the KSZ9021. Shame Micrel, Shame!
 */
static struct phy_driver KS8721_driver = {
	.name = "Micrel KS8721BL",
	.uid = 0x221610,
	.mask = 0xfffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};
#endif


/*
 * KSZ9021 - KSZ9031 common
 */

#define MII_KSZ90xx_PHY_CTL		0x1f
#define MIIM_KSZ90xx_PHYCTL_1000	(1 << 6)
#define MIIM_KSZ90xx_PHYCTL_100		(1 << 5)
#define MIIM_KSZ90xx_PHYCTL_10		(1 << 4)
#define MIIM_KSZ90xx_PHYCTL_DUPLEX	(1 << 3)

static int ksz90xx_startup(struct phy_device *phydev)
{
	unsigned phy_ctl;
	genphy_update_link(phydev);
	phy_ctl = phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZ90xx_PHY_CTL);

	if (phy_ctl & MIIM_KSZ90xx_PHYCTL_DUPLEX)
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	if (phy_ctl & MIIM_KSZ90xx_PHYCTL_1000)
		phydev->speed = SPEED_1000;
	else if (phy_ctl & MIIM_KSZ90xx_PHYCTL_100)
		phydev->speed = SPEED_100;
	else if (phy_ctl & MIIM_KSZ90xx_PHYCTL_10)
		phydev->speed = SPEED_10;
	return 0;
}

/* Common OF config bits for KSZ9021 and KSZ9031 */
#if defined(CONFIG_PHY_MICREL_KSZ9021) || defined(CONFIG_PHY_MICREL_KSZ9031)
#ifdef CONFIG_DM_ETH
struct ksz90x1_reg_field {
	const char	*name;
	const u8	size;	/* Size of the bitfield, in bits */
	const u8	off;	/* Offset from bit 0 */
	const u8	dflt;	/* Default value */
};

struct ksz90x1_ofcfg {
	const u16			reg;
	const u16			devad;
	const struct ksz90x1_reg_field	*grp;
	const u16			grpsz;
};

static const struct ksz90x1_reg_field ksz90x1_rxd_grp[] = {
	{ "rxd0-skew-ps", 4, 0, 0x7 }, { "rxd1-skew-ps", 4, 4, 0x7 },
	{ "rxd2-skew-ps", 4, 8, 0x7 }, { "rxd3-skew-ps", 4, 12, 0x7 }
};

static const struct ksz90x1_reg_field ksz90x1_txd_grp[] = {
	{ "txd0-skew-ps", 4, 0, 0x7 }, { "txd1-skew-ps", 4, 4, 0x7 },
	{ "txd2-skew-ps", 4, 8, 0x7 }, { "txd3-skew-ps", 4, 12, 0x7 },
};

static int ksz90x1_of_config_group(struct phy_device *phydev,
				   struct ksz90x1_ofcfg *ofcfg)
{
	struct udevice *dev = phydev->dev;
	struct phy_driver *drv = phydev->drv;
	const int ps_to_regval = 200;
	int val[4];
	int i, changed = 0, offset, max;
	u16 regval = 0;

	if (!drv || !drv->writeext)
		return -EOPNOTSUPP;

	for (i = 0; i < ofcfg->grpsz; i++) {
		val[i] = fdtdec_get_uint(gd->fdt_blob, dev->of_offset,
					 ofcfg->grp[i].name, -1);
		offset = ofcfg->grp[i].off;
		if (val[i] == -1) {
			/* Default register value for KSZ9021 */
			regval |= ofcfg->grp[i].dflt << offset;
		} else {
			changed = 1;	/* Value was changed in OF */
			/* Calculate the register value and fix corner cases */
			if (val[i] > ps_to_regval * 0xf) {
				max = (1 << ofcfg->grp[i].size) - 1;
				regval |= max << offset;
			} else {
				regval |= (val[i] / ps_to_regval) << offset;
			}
		}
	}

	if (!changed)
		return 0;

	return drv->writeext(phydev, 0, ofcfg->devad, ofcfg->reg, regval);
}
#endif
#endif

#ifdef CONFIG_PHY_MICREL_KSZ9021
/*
 * KSZ9021
 */

/* PHY Registers */
#define MII_KSZ9021_EXTENDED_CTRL	0x0b
#define MII_KSZ9021_EXTENDED_DATAW	0x0c
#define MII_KSZ9021_EXTENDED_DATAR	0x0d

#define CTRL1000_PREFER_MASTER		(1 << 10)
#define CTRL1000_CONFIG_MASTER		(1 << 11)
#define CTRL1000_MANUAL_CONFIG		(1 << 12)

#ifdef CONFIG_DM_ETH
static const struct ksz90x1_reg_field ksz9021_clk_grp[] = {
	{ "txen-skew-ps", 4, 0, 0x7 }, { "txc-skew-ps", 4, 4, 0x7 },
	{ "rxdv-skew-ps", 4, 8, 0x7 }, { "rxc-skew-ps", 4, 12, 0x7 },
};

static int ksz9021_of_config(struct phy_device *phydev)
{
	struct ksz90x1_ofcfg ofcfg[] = {
		{ MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, 0, ksz90x1_rxd_grp, 4 },
		{ MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, 0, ksz90x1_txd_grp, 4 },
		{ MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, 0, ksz9021_clk_grp, 4 },
	};
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(ofcfg); i++)
		ret = ksz90x1_of_config_group(phydev, &(ofcfg[i]));
		if (ret)
			return ret;

	return 0;
}
#else
static int ksz9021_of_config(struct phy_device *phydev)
{
	return 0;
}
#endif

int ksz9021_phy_extended_write(struct phy_device *phydev, int regnum, u16 val)
{
	/* extended registers */
	phy_write(phydev, MDIO_DEVAD_NONE,
		MII_KSZ9021_EXTENDED_CTRL, regnum | 0x8000);
	return phy_write(phydev, MDIO_DEVAD_NONE,
		MII_KSZ9021_EXTENDED_DATAW, val);
}

int ksz9021_phy_extended_read(struct phy_device *phydev, int regnum)
{
	/* extended registers */
	phy_write(phydev, MDIO_DEVAD_NONE, MII_KSZ9021_EXTENDED_CTRL, regnum);
	return phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZ9021_EXTENDED_DATAR);
}


static int ksz9021_phy_extread(struct phy_device *phydev, int addr, int devaddr,
			      int regnum)
{
	return ksz9021_phy_extended_read(phydev, regnum);
}

static int ksz9021_phy_extwrite(struct phy_device *phydev, int addr,
			       int devaddr, int regnum, u16 val)
{
	return ksz9021_phy_extended_write(phydev, regnum, val);
}

/* Micrel ksz9021 */
static int ksz9021_config(struct phy_device *phydev)
{
	unsigned ctrl1000 = 0;
	const unsigned master = CTRL1000_PREFER_MASTER |
			CTRL1000_CONFIG_MASTER | CTRL1000_MANUAL_CONFIG;
	unsigned features = phydev->drv->features;
	int ret;

	ret = ksz9021_of_config(phydev);
	if (ret)
		return ret;

	if (getenv("disable_giga"))
		features &= ~(SUPPORTED_1000baseT_Half |
				SUPPORTED_1000baseT_Full);
	/* force master mode for 1000BaseT due to chip errata */
	if (features & SUPPORTED_1000baseT_Half)
		ctrl1000 |= ADVERTISE_1000HALF | master;
	if (features & SUPPORTED_1000baseT_Full)
		ctrl1000 |= ADVERTISE_1000FULL | master;
	phydev->advertising = phydev->supported = features;
	phy_write(phydev, MDIO_DEVAD_NONE, MII_CTRL1000, ctrl1000);
	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);
	return 0;
}

static struct phy_driver ksz9021_driver = {
	.name = "Micrel ksz9021",
	.uid  = 0x221610,
	.mask = 0xfffff0,
	.features = PHY_GBIT_FEATURES,
	.config = &ksz9021_config,
	.startup = &ksz90xx_startup,
	.shutdown = &genphy_shutdown,
	.writeext = &ksz9021_phy_extwrite,
	.readext = &ksz9021_phy_extread,
};
#endif

/**
 * KSZ9031
 */
/* PHY Registers */
#define MII_KSZ9031_MMD_ACCES_CTRL	0x0d
#define MII_KSZ9031_MMD_REG_DATA	0x0e

#ifdef CONFIG_DM_ETH
static const struct ksz90x1_reg_field ksz9031_ctl_grp[] =
	{ { "txen-skew-ps", 4, 0, 0x7 }, { "rxdv-skew-ps", 4, 4, 0x7 } };
static const struct ksz90x1_reg_field ksz9031_clk_grp[] =
	{ { "rxc-skew-ps", 5, 0, 0xf }, { "txc-skew-ps", 5, 5, 0xf } };

static int ksz9031_of_config(struct phy_device *phydev)
{
	struct ksz90x1_ofcfg ofcfg[] = {
		{ MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW, 2, ksz9031_ctl_grp, 2 },
		{ MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW, 2, ksz90x1_rxd_grp, 4 },
		{ MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW, 2, ksz90x1_txd_grp, 4 },
		{ MII_KSZ9031_EXT_RGMII_CLOCK_SKEW, 2, ksz9031_clk_grp, 2 },
	};
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(ofcfg); i++)
		ret = ksz90x1_of_config_group(phydev, &(ofcfg[i]));
		if (ret)
			return ret;

	return 0;
}
#else
static int ksz9031_of_config(struct phy_device *phydev)
{
	return 0;
}
#endif

/* Accessors to extended registers*/
int ksz9031_phy_extended_write(struct phy_device *phydev,
			       int devaddr, int regnum, u16 mode, u16 val)
{
	/*select register addr for mmd*/
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MII_KSZ9031_MMD_ACCES_CTRL, devaddr);
	/*select register for mmd*/
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MII_KSZ9031_MMD_REG_DATA, regnum);
	/*setup mode*/
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MII_KSZ9031_MMD_ACCES_CTRL, (mode | devaddr));
	/*write the value*/
	return	phy_write(phydev, MDIO_DEVAD_NONE,
		MII_KSZ9031_MMD_REG_DATA, val);
}

int ksz9031_phy_extended_read(struct phy_device *phydev, int devaddr,
			      int regnum, u16 mode)
{
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MII_KSZ9031_MMD_ACCES_CTRL, devaddr);
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MII_KSZ9031_MMD_REG_DATA, regnum);
	phy_write(phydev, MDIO_DEVAD_NONE,
		  MII_KSZ9031_MMD_ACCES_CTRL, (devaddr | mode));
	return phy_read(phydev, MDIO_DEVAD_NONE, MII_KSZ9031_MMD_REG_DATA);
}

static int ksz9031_phy_extread(struct phy_device *phydev, int addr, int devaddr,
			       int regnum)
{
	return ksz9031_phy_extended_read(phydev, devaddr, regnum,
					 MII_KSZ9031_MOD_DATA_NO_POST_INC);
};

static int ksz9031_phy_extwrite(struct phy_device *phydev, int addr,
				int devaddr, int regnum, u16 val)
{
	return ksz9031_phy_extended_write(phydev, devaddr, regnum,
					 MII_KSZ9031_MOD_DATA_POST_INC_RW, val);
};

static int ksz9031_config(struct phy_device *phydev)
{
	int ret;
	ret = ksz9031_of_config(phydev);
	if (ret)
		return ret;
	return genphy_config(phydev);
}

static struct phy_driver ksz9031_driver = {
	.name = "Micrel ksz9031",
	.uid  = 0x221620,
	.mask = 0xfffff0,
	.features = PHY_GBIT_FEATURES,
	.config   = &ksz9031_config,
	.startup  = &ksz90xx_startup,
	.shutdown = &genphy_shutdown,
	.writeext = &ksz9031_phy_extwrite,
	.readext = &ksz9031_phy_extread,
};

int phy_micrel_init(void)
{
	phy_register(&KSZ804_driver);
	phy_register(&KSZ8031_driver);
	phy_register(&KSZ8051_driver);
	phy_register(&KSZ8081_driver);
#ifdef CONFIG_PHY_MICREL_KSZ9021
	phy_register(&ksz9021_driver);
#else
	phy_register(&KS8721_driver);
#endif
	phy_register(&ksz9031_driver);
	phy_register(&ksz8895_driver);
	return 0;
}

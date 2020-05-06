// SPDX-License-Identifier: GPL-2.0+
/*
 * Atheros PHY drivers
 *
 * Copyright 2011, 2013 Freescale Semiconductor, Inc.
 * author Andy Fleming
 */
#include <common.h>
#include <phy.h>

#define AR803x_PHY_DEBUG_ADDR_REG	0x1d
#define AR803x_PHY_DEBUG_DATA_REG	0x1e

#define AR803x_DEBUG_REG_5		0x5
#define AR803x_RGMII_TX_CLK_DLY		BIT(8)

#define AR803x_DEBUG_REG_0		0x0
#define AR803x_RGMII_RX_CLK_DLY		BIT(15)

/* CLK_25M register is at MMD 7, address 0x8016 */
#define AR803x_CLK_25M_SEL_REG		0x8016
/* AR8035: Select frequency on CLK_25M pin through bits 4:3 */
#define AR8035_CLK_25M_FREQ_25M		(0 | 0)
#define AR8035_CLK_25M_FREQ_50M		(0 | BIT(3))
#define AR8035_CLK_25M_FREQ_62M		(BIT(4) | 0)
#define AR8035_CLK_25M_FREQ_125M	(BIT(4) | BIT(3))
#define AR8035_CLK_25M_MASK		GENMASK(4, 3)

#define AR8021_PHY_ID 0x004dd040
#define AR8031_PHY_ID 0x004dd074
#define AR8035_PHY_ID 0x004dd072

static int ar803x_debug_reg_read(struct phy_device *phydev, u16 reg)
{
	int ret;

	ret = phy_write(phydev, MDIO_DEVAD_NONE, AR803x_PHY_DEBUG_ADDR_REG,
			reg);
	if (ret < 0)
		return ret;

	return phy_read(phydev, MDIO_DEVAD_NONE, AR803x_PHY_DEBUG_DATA_REG);
}

static int ar803x_debug_reg_mask(struct phy_device *phydev, u16 reg,
				 u16 clear, u16 set)
{
	int val;

	val = ar803x_debug_reg_read(phydev, reg);
	if (val < 0)
		return val;

	val &= 0xffff;
	val &= ~clear;
	val |= set;

	return phy_write(phydev, MDIO_DEVAD_NONE, AR803x_PHY_DEBUG_DATA_REG,
			 val);
}

static int ar803x_enable_rx_delay(struct phy_device *phydev, bool on)
{
	u16 clear = 0, set = 0;

	if (on)
		set = AR803x_RGMII_RX_CLK_DLY;
	else
		clear = AR803x_RGMII_RX_CLK_DLY;

	return ar803x_debug_reg_mask(phydev, AR803x_DEBUG_REG_0, clear, set);
}

static int ar803x_enable_tx_delay(struct phy_device *phydev, bool on)
{
	u16 clear = 0, set = 0;

	if (on)
		set = AR803x_RGMII_TX_CLK_DLY;
	else
		clear = AR803x_RGMII_TX_CLK_DLY;

	return ar803x_debug_reg_mask(phydev, AR803x_DEBUG_REG_5, clear, set);
}

static int ar8021_config(struct phy_device *phydev)
{
	phy_write(phydev, MDIO_DEVAD_NONE, MII_BMCR,
		  BMCR_ANENABLE | BMCR_ANRESTART);

	ar803x_enable_tx_delay(phydev, true);

	phydev->supported = phydev->drv->features;
	return 0;
}

static int ar803x_delay_config(struct phy_device *phydev)
{
	int ret;

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		ret = ar803x_enable_tx_delay(phydev, true);
	else
		ret = ar803x_enable_tx_delay(phydev, false);

	if (phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID ||
	    phydev->interface == PHY_INTERFACE_MODE_RGMII_ID)
		ret = ar803x_enable_rx_delay(phydev, true);
	else
		ret = ar803x_enable_rx_delay(phydev, false);

	return ret;
}

static int ar8031_config(struct phy_device *phydev)
{
	int ret;

	ret = ar803x_delay_config(phydev);
	if (ret < 0)
		return ret;

	phydev->supported = phydev->drv->features;

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

static int ar8035_config(struct phy_device *phydev)
{
	int ret;
	int regval;

	/* Configure CLK_25M output clock at 125 MHz */
	regval = phy_read_mmd(phydev, MDIO_MMD_AN, AR803x_CLK_25M_SEL_REG);
	regval &= ~AR8035_CLK_25M_MASK; /* No surprises */
	regval |= AR8035_CLK_25M_FREQ_125M;
	phy_write_mmd(phydev, MDIO_MMD_AN, AR803x_CLK_25M_SEL_REG, regval);

	ret = ar803x_delay_config(phydev);
	if (ret < 0)
		return ret;

	phydev->supported = phydev->drv->features;

	genphy_config_aneg(phydev);
	genphy_restart_aneg(phydev);

	return 0;
}

static struct phy_driver AR8021_driver =  {
	.name = "AR8021",
	.uid = AR8021_PHY_ID,
	.mask = 0xfffffff0,
	.features = PHY_GBIT_FEATURES,
	.config = ar8021_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

static struct phy_driver AR8031_driver =  {
	.name = "AR8031/AR8033",
	.uid = AR8031_PHY_ID,
	.mask = 0xffffffef,
	.features = PHY_GBIT_FEATURES,
	.config = ar8031_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

static struct phy_driver AR8035_driver =  {
	.name = "AR8035",
	.uid = AR8035_PHY_ID,
	.mask = 0xffffffef,
	.features = PHY_GBIT_FEATURES,
	.config = ar8035_config,
	.startup = genphy_startup,
	.shutdown = genphy_shutdown,
};

int phy_atheros_init(void)
{
	phy_register(&AR8021_driver);
	phy_register(&AR8031_driver);
	phy_register(&AR8035_driver);

	return 0;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * TI Generic PHY Init to register any TI Ethernet PHYs
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * Copyright (C) 2019-20 Texas Instruments Inc.
 */

#include <phy.h>
#include "ti_phy_init.h"

#define DP83822_DEVADDR		0x1f

#define MII_DP83822_RCSR	0x17

/* RCSR bits */
#define DP83822_RX_CLK_SHIFT	BIT(12)
#define DP83822_TX_CLK_SHIFT	BIT(11)

/* DP83822 specific RGMII RX/TX delay configuration. */
static int dp83822_config(struct phy_device *phydev)
{
	ofnode node = phy_get_ofnode(phydev);
	u32 rgmii_delay = 0;
	u32 rx_delay = 0;
	u32 tx_delay = 0;
	int ret;

	ret = ofnode_read_u32(node, "rx-internal-delay-ps", &rx_delay);
	if (ret) {
		rx_delay = phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
			   phydev->interface == PHY_INTERFACE_MODE_RGMII_RXID;
	}

	ret = ofnode_read_u32(node, "tx-internal-delay-ps", &tx_delay);
	if (ret) {
		tx_delay = phydev->interface == PHY_INTERFACE_MODE_RGMII_ID ||
			   phydev->interface == PHY_INTERFACE_MODE_RGMII_TXID;
	}

	/* Bit set means Receive path internal clock shift is ENABLED */
	if (rx_delay)
		rgmii_delay |= DP83822_RX_CLK_SHIFT;

	/* Bit set means Transmit path internal clock shift is DISABLED */
	if (!tx_delay)
		rgmii_delay |= DP83822_TX_CLK_SHIFT;

	ret = phy_modify_mmd(phydev, DP83822_DEVADDR, MII_DP83822_RCSR,
			     DP83822_RX_CLK_SHIFT | DP83822_TX_CLK_SHIFT,
			     rgmii_delay);
	if (ret)
		return ret;

	return genphy_config_aneg(phydev);
}

U_BOOT_PHY_DRIVER(dp83822) = {
	.name = "TI DP83822",
	.uid = 0x2000a240,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &dp83822_config,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83826nc) = {
	.name = "TI DP83826NC",
	.uid = 0x2000a110,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83826c) = {
	.name = "TI DP83826C",
	.uid = 0x2000a130,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825s) = {
	.name = "TI DP83825S",
	.uid = 0x2000a140,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825i) = {
	.name = "TI DP83825I",
	.uid = 0x2000a150,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825m) = {
	.name = "TI DP83825M",
	.uid = 0x2000a160,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

U_BOOT_PHY_DRIVER(dp83825cs) = {
	.name = "TI DP83825CS",
	.uid = 0x2000a170,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

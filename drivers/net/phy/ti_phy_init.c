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

#ifdef CONFIG_PHY_TI_GENERIC
static struct phy_driver dp83822_driver = {
	.name = "TI DP83822",
	.uid = 0x2000a240,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver dp83826nc_driver = {
	.name = "TI DP83826NC",
	.uid = 0x2000a110,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver dp83826c_driver = {
	.name = "TI DP83826C",
	.uid = 0x2000a130,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver dp83825s_driver = {
	.name = "TI DP83825S",
	.uid = 0x2000a140,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver dp83825i_driver = {
	.name = "TI DP83825I",
	.uid = 0x2000a150,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver dp83825m_driver = {
	.name = "TI DP83825M",
	.uid = 0x2000a160,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};

static struct phy_driver dp83825cs_driver = {
	.name = "TI DP83825CS",
	.uid = 0x2000a170,
	.mask = 0xfffffff0,
	.features = PHY_BASIC_FEATURES,
	.config = &genphy_config_aneg,
	.startup = &genphy_startup,
	.shutdown = &genphy_shutdown,
};
#endif /* CONFIG_PHY_TI_GENERIC */

int phy_ti_init(void)
{
#ifdef CONFIG_PHY_TI_DP83867
	phy_dp83867_init();
#endif

#ifdef CONFIG_PHY_TI_GENERIC
	phy_register(&dp83822_driver);
	phy_register(&dp83825s_driver);
	phy_register(&dp83825i_driver);
	phy_register(&dp83825m_driver);
	phy_register(&dp83825cs_driver);
	phy_register(&dp83826c_driver);
	phy_register(&dp83826nc_driver);
#endif
	return 0;
}

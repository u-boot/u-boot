// SPDX-License-Identifier: GPL-2.0
/*
 * TI Generic PHY Init to register any TI Ethernet PHYs
 *
 * Author: Dan Murphy <dmurphy@ti.com>
 *
 * Copyright (C) 2019-20 Texas Instruments Inc.
 */

#include "ti_phy_init.h"

int phy_ti_init(void)
{
#ifdef CONFIG_PHY_TI_DP83867
	phy_dp83867_init();
#endif
	return 0;
}

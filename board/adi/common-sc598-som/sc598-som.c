// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025 - Analog Devices, Inc.
 */

#include <config.h>
#include <phy.h>
#include <asm/u-boot.h>
#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/arch-adi/sc5xx/soc.h>
#include <asm/armv8/mmu.h>

#include "../carriers/somcrr.h"

int board_phy_config(struct phy_device *phydev)
{
	if (IS_ENABLED(CONFIG_ADI_CARRIER_SOMCRR_EZKIT))
		fixup_dp83867_phy(phydev);
	return 0;
}

int board_init(void)
{
	sc59x_remap_ospi();

	if (IS_ENABLED(CONFIG_ADI_CARRIER_SOMCRR_EZKIT) ||
	    IS_ENABLED(CONFIG_ADI_CARRIER_SOMCRR_EZLITE)) {
		adi_somcrr_init_ethernet();
	}

	sc5xx_enable_rgmii();

	return 0;
}

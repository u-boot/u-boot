// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2025 - Analog Devices, Inc.
 */

#include <phy.h>
#include <asm/u-boot.h>
#include <asm/arch-adi/sc5xx/sc5xx.h>
#include <asm/arch-adi/sc5xx/soc.h>

int board_phy_config(struct phy_device *phydev)
{
	fixup_dp83867_phy(phydev);
	return 0;
}

int board_init(void)
{
	sc5xx_enable_rgmii();
	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2017 Theobroma Systems Design und Consulting GmbH
 */

#include "../common/common.h"

int rockchip_early_misc_init_r(void)
{
	setup_boottargets();

	return 0;
}

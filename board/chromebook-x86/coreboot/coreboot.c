/*
 * Copyright (C) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <cros_ec.h>

int arch_early_init_r(void)
{
	if (cros_ec_board_init())
		return -1;

	return 0;
}

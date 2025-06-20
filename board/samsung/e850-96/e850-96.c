// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024, Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 */

#include <init.h>
#include "fw.h"

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int board_init(void)
{
	return 0;
}

int board_late_init(void)
{
	int err;

	/*
	 * Do this in board_late_init() to make sure MMC is not probed before
	 * efi_init_early().
	 */
	err = load_ldfw();
	if (err)
		printf("ERROR: LDFW loading failed (%d)\n", err);

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024, Texas Instruments Incorporated - https://www.ti.com/
 */

#include <fdt_support.h>
#include <dm/uclass.h>
#include <k3-ddrss.h>
#include <spl.h>

#include "k3-ddr.h"

int dram_init(void)
{
	s32 ret;

	ret = fdtdec_setup_mem_size_base_lowest();
	if (ret)
		printf("Error setting up mem size and base. %d\n", ret);

	return ret;
}

int dram_init_banksize(void)
{
	s32 ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		printf("Error setting up memory banksize. %d\n", ret);

	return ret;
}

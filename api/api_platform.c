// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
 *
 * This file contains a routine to fetch data from the global_data structure.
 */

#include <api_public.h>
#include <asm/global_data.h>
#include "api_private.h"

DECLARE_GLOBAL_DATA_PTR;

int platform_sys_info(struct sys_info *si)
{
	int i;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++)
		platform_set_mr(si, gd->bd->bi_dram[i].start,
				gd->bd->bi_dram[i].size, MR_ATTR_DRAM);

	return 1;
}

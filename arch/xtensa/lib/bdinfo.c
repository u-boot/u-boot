// SPDX-License-Identifier: GPL-2.0+
/*
 * XTENSA-specific information for the 'bd' command
 *
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <init.h>

DECLARE_GLOBAL_DATA_PTR;

int arch_setup_bdinfo(void)
{
	struct bd_info *bd = gd->bd;

	bd->bi_memstart = PHYSADDR(CONFIG_SYS_SDRAM_BASE);
	bd->bi_memsize = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

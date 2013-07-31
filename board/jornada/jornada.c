/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * 2004 (c) MontaVista Software, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <SA-1100.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

int board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_JORNADA720;
	gd->bd->bi_boot_params = 0xc0000100;


	/*
	 * Turn on flashing.
	 * Would be nice to have some protection but
	 * that would have to be implemented in the
	 * flash init function, which isnt possible yet.
	 */
	PPSR |= (1 << 7);
	PPDR |= (1 << 7);

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return (0);
}

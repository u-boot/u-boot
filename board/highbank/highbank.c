/*
 * Copyright 2010-2011 Calxeda, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <ahci.h>
#include <scsi.h>

#include <asm/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscellaneous platform dependent initialisations
 */
int board_init(void)
{
	icache_enable();

	return 0;
}

int misc_init_r(void)
{
	ahci_init(0xffe08000);
	scsi_scan(1);
	return 0;
}

int dram_init(void)
{
	gd->ram_size = SZ_512M;
	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  PHYS_SDRAM_1_SIZE;
}

void reset_cpu(ulong addr)
{
}

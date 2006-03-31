/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2005 Rowel Atienza <rowel@diwalabs.com>
 * Armadillo board HT1070
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <clps7111.h>

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */


/*
 * Miscelaneous platform dependent initialisations
 */

int board_init (void)
{
	/* Activate LED flasher */
	IO_LEDFLSH = 0x40;

	/* arch number MACH_TYPE_ARMADILLO - not official*/
	gd->bd->bi_arch_number = 83;

	/* location of boot parameters */
	gd->bd->bi_boot_params = 0xc0000100;

	return 0;
}

int dram_init (void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	return (0);
}

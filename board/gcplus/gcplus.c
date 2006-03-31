/*
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * 2003-2004 (c) MontaVista Software, Inc.
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
#include <SA-1100.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Miscelaneous platform dependent initialisations
 */

int
board_init(void)
{
	gd->bd->bi_arch_number = MACH_TYPE_GRAPHICSCLIENT;

	gd->bd->bi_boot_params = 0xc000003c; /* Weird address? */

	/* Most of the ADS GCPlus I/O is connected to Static nCS2.
	 * So I'm brute forcing nCS2 timiming here for worst case.
	 */
	MSC1 &= ~0xFFFF;
	MSC1 |= 0x8649;

	/* Nothing is connected to Static nCS4 or nCS5. But I'm using
	 * nCS4 as a paranoia safe guard to force nCS2, nOE; nWE high
	 * after accessing I/O via (non-VLIO) nCS2. What can I say, I'm
	 * paranoid and lack decent tools to alleviate my fear. I sure
	 * do wish I had a logic analyzer. : (
	 */

	MSC2 =  0xfff9fff9;

	return 0;
}

int
dram_init(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

	return (0);
}

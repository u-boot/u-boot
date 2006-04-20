/*
 * Copyright (C) 2003 ETC s.r.o.
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
 *
 * Written by Peter Figuli <peposh@etc.sk>, 2003.
 *
 */

#include <common.h>
#include <asm/arch/pxa-regs.h>

DECLARE_GLOBAL_DATA_PTR;

int board_init (void)
{
	gd->bd->bi_arch_number = MACH_TYPE_WEP_EP250;
	gd->bd->bi_boot_params = 0xa0000000;
/*
 * Setup GPIO stuff to get serial working
 */
#if defined( CONFIG_FFUART )
	GPDR1 = 0x80;
	GAFR1_L = 0x8010;
#elif defined( CONFIG_BTUART )
	GPDR1 = 0x800;
	GAFR1_L = 0x900000;
#endif
	PSSR = 0x20;

	return 0;
}

int dram_init (void)
{
#if ( CONFIG_NR_DRAM_BANKS > 0 )
	gd->bd->bi_dram[0].start = WEP_SDRAM_1;
	gd->bd->bi_dram[0].size = WEP_SDRAM_1_SIZE;
#endif
#if ( CONFIG_NR_DRAM_BANKS > 1 )
	gd->bd->bi_dram[1].start = WEP_SDRAM_2;
	gd->bd->bi_dram[1].size = WEP_SDRAM_2_SIZE;
#endif
#if ( CONFIG_NR_DRAM_BANKS > 2 )
	gd->bd->bi_dram[2].start = WEP_SDRAM_3;
	gd->bd->bi_dram[2].size = WEP_SDRAM_3_SIZE;
#endif
#if ( CONFIG_NR_DRAM_BANKS > 3 )
	gd->bd->bi_dram[3].start = WEP_SDRAM_4;
	gd->bd->bi_dram[3].size = WEP_SDRAM_4_SIZE;
#endif

	return 0;
}

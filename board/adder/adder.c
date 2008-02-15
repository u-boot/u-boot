/*
 * Copyright (C) 2004-2005 Arabella Software Ltd.
 * Yuli Barcohen <yuli@arabellasw.com>
 *
 * Support for Analogue&Micro Adder boards family.
 * Tested on AdderII and Adder87x.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <mpc8xx.h>
#if defined(CONFIG_OF_LIBFDT)
	#include <libfdt.h>
#endif

/*
 * SDRAM is single Samsung K4S643232F-T70   chip (8MB)
 *       or single Micron  MT48LC4M32B2TG-7 chip (16MB).
 * Minimal CPU frequency is 40MHz.
 */
static uint sdram_table[] = {
	/* Single read	(offset 0x00 in UPM RAM) */
	0x1f07fc24, 0xe0aefc04, 0x10adfc04, 0xe0bbbc00,
	0x10f77c44, 0xf3fffc07, 0xfffffc04, 0xfffffc04,

	/* Burst read	(offset 0x08 in UPM RAM) */
	0x1f07fc24, 0xe0aefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf0affc00, 0xf0affc00, 0x10a77c44,
	0xf7bffc47, 0xfffffc35, 0xfffffc34, 0xfffffc35,
	0xfffffc35, 0x1ff77c35, 0xfffffc34, 0x1fb57c35,

	/* Single write (offset 0x18 in UPM RAM) */
	0x1f27fc24, 0xe0aebc04, 0x00b93c00, 0x13f77c47,
	0xfffdfc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* Burst write	(offset 0x20 in UPM RAM) */
	0x1f07fc24, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe0abbc00, 0x1fb77c47, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* Refresh	(offset 0x30 in UPM RAM) */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, 0xfffffc04, 0xfffffc04,
	0xfffffc04, 0xfffffc04, 0xfffffc04, 0xfffffc04,

	/* Exception	(offset 0x3C in UPM RAM) */
	0xfffffc27, 0xfffffc04, 0xfffffc04, 0xfffffc04
};

long int initdram (int board_type)
{
	long int msize;
	volatile immap_t     *immap  = (volatile immap_t *)CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	upmconfig(UPMA, sdram_table, sizeof(sdram_table) / sizeof(uint));

	/* Configure SDRAM refresh */
	memctl->memc_mptpr = MPTPR_PTP_DIV32; /* BRGCLK/32 */

	memctl->memc_mamr = (94 << 24) | CFG_MAMR; /* No refresh */
	udelay(200);

	/* Run precharge from location 0x15 */
	memctl->memc_mar = 0x0;
	memctl->memc_mcr = 0x80002115;
	udelay(200);

	/* Run 8 refresh cycles */
	memctl->memc_mcr = 0x80002830;
	udelay(200);

	/* Run MRS pattern from location 0x16 */
	memctl->memc_mar = 0x88;
	memctl->memc_mcr = 0x80002116;
	udelay(200);

	memctl->memc_mamr |=  MAMR_PTAE; /* Enable refresh */
	memctl->memc_or1   = ~(CFG_SDRAM_MAX_SIZE - 1) | OR_CSNT_SAM;
	memctl->memc_br1   =  CFG_SDRAM_BASE | BR_PS_32 | BR_MS_UPMA | BR_V;

	msize = get_ram_size(CFG_SDRAM_BASE, CFG_SDRAM_MAX_SIZE);
	memctl->memc_or1  |= ~(msize - 1);

	return msize;
}

int checkboard( void )
{
	puts("Board: Adder");
#if defined(CONFIG_MPC885_FAMILY)
	puts("87x\n");
#elif defined(CONFIG_MPC866_FAMILY)
	puts("II\n");
#endif

	return 0;
}

#if defined(CONFIG_OF_BOARD_SETUP)
void ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

}
#endif

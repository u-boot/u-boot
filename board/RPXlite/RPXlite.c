/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * Yoo. Jonghoon, IPone, yooth@ipone.co.kr
 * U-Boot port on RPXlite board
 *
 * DRAM related UPMA register values are modified.
 * See RPXLite engineering note : 50MHz/60ns - UPM RAM WORDS
 */

#include <common.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFCC25

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 00h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0X0CAFCC04, 0X03AFCC08,
	0x3FBFCC27,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Read. (Offset 08h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0x0CAFCC84, 0x03AFCC88,
	0x3FBFCC27,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0x0CFFCC04, 0x03FFCC00,
	0x3FFFCC27,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Write. (Offset 20h in UPMA RAM)
	 */
	0xCFFFCC24, 0x0FFFCC04, 0x0CFFCC80, 0x03FFCC8C,
	0x0CFFCC00, 0x33FFCC27,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/*
	 * Refresh. (Offset 30h in UPMA RAM)
	 */
	0xC0FFCC24, 0x03FFCC24, 0x0FFFCC24, 0x0FFFCC24,
	0x3FFFCC27,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Exception. (Offset 3Ch in UPMA RAM)
	 */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: RPXlite\n");
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size10;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/* Refresh clock prescalar */
	memctl->memc_mptpr = CFG_MPTPR;

	memctl->memc_mar = 0x00000000;

	/* Map controller banks 1 to the SDRAM bank */
	memctl->memc_or1 = CFG_OR1_PRELIM;
	memctl->memc_br1 = CFG_BR1_PRELIM;

	memctl->memc_mamr = CFG_MAMR_10COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80002230;	/* SDRAM bank 0 - refresh twice */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/* Check Bank 0 Memory Size
	 * try 10 column mode
	 */

	size10 = dram_size (CFG_MAMR_10COL, SDRAM_BASE_PRELIM,
			    SDRAM_MAX_SIZE);

	return (size10);
}

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'. Some (not all) hardware errors are detected:
 * - short between address lines
 * - short between data lines
 */

static long int dram_size (long int mamr_value, long int *base,
			   long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size (base, maxsize));
}

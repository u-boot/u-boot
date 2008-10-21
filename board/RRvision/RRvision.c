/*
 * (C) Copyright 2001-2002
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

#include <common.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x11ADFC04, 0xEFBBBC00,
	0x1FF77C47, /* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
		    0x1FF77C34, 0xEFEABC34, 0x1FB57C35, /* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEFC04, 0x10ADFC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBBC00, 0x1FF77C47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAEBC00, 0x10AD7C00, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBBC04, 0x1FF77C47, /* last */
					    _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FF5FC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07, /* last */
				_NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Always return 1 (no second DRAM bank).
 */

int checkboard (void)
{
	char *s = getenv ("serial#");

	puts ("Board: RRvision ");

	for (; s && *s; ++s) {
		if (*s == ' ')
			break;
		putc (*s);
	}

	putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	unsigned long reg;
	long int size8, size9;
	long int size = 0;

	upmconfig (UPMA, (uint *)sdram_table, sizeof(sdram_table) / sizeof(uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 1 the SDRAM bank 2 at physical address 0.
	 */
	memctl->memc_or1 = CONFIG_SYS_OR2_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR2_PRELIM;

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80002105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80002230;	/* SDRAM bank 0 - execute twice */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL,
			   SDRAM_BASE2_PRELIM,
			   SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL,
			   SDRAM_BASE2_PRELIM,
			   SDRAM_MAX_SIZE);

	if (size8 < size9) {		/* leave configuration at 9 columns */
		size = size9;
/*		debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
	} else {			/* back to 8 columns            */
		size = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
/*		debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
	}

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if (size < 0x02000000) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping
	 */
	memctl->memc_or1 = ((-size) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
	memctl->memc_br1 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	/*
	 * No bank 1
	 *
	 * invalidate bank
	 */
	memctl->memc_br3 = 0;

	/* adjust refresh rate depending on SDRAM type, one bank */
	reg = memctl->memc_mptpr;
	reg >>= 1;			/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
	memctl->memc_mptpr = reg;

	udelay (10000);

	return (size);
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
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}

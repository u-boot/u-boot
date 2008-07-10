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

#include <common.h>
#include "mpc8xx.h"
#include <linux/mtd/doc2000.h>

extern int kbd_init(void);
extern int drv_kbd_init(void);

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
	0x1F27FC04, 0xEEAEBC00, 0x01B93C04, 0x1FF77C47, /* last */
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
	0x1FF7FC07, /* last */
		    _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

const uint static_table[] =
{
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x0FFFFC04, 0x0FF3FC04, 0x0FF3CC04, 0x0FF3CC04,
	0x0FF3EC04, 0x0FF3CC00, 0x0FF7FC04, 0x3FFFFC04,
	0xFFFFFC04, 0xFFFFFC05, /* last */
	                        _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x0FFFFC04, 0x00FFFC04, 0x00FFFC04, 0x00FFFC04,
	0x01FFFC00, 0x3FFFFC04, 0xFFFFFC04, 0xFFFFFC05, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 *
 * Test TQ ID string (TQM8xx...)
 * If present, check for "L" type (no second DRAM bank),
 * otherwise "L" type is assumed as default.
 *
 * Return 1 for "L" type, 0 else.
 */

int checkboard (void)
{
	char *s = getenv ("serial#");

	if (!s || strncmp (s, "TQM8", 4)) {
		printf ("### No HW ID - assuming RBC823\n");
		return (0);
	}

	puts (s);
	putc ('\n');

	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0, size8, size9;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/*
	 * 1 Bank of 64Mbit x 2 devices
	 */
	memctl->memc_mptpr = CFG_MPTPR_1BK_4K;
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller SDRAM bank 0
	 */
	memctl->memc_or4 = CFG_OR4_PRELIM;
	memctl->memc_br4 = CFG_BR4_PRELIM;
	memctl->memc_mamr = CFG_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */
	udelay (200);

	/*
	 * Perform SDRAM initializsation sequence
	 */
	memctl->memc_mcr = 0x80008105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mamr = (CFG_MAMR_8COL & ~(MAMR_TLFA_MSK)) | MAMR_TLFA_8X;
	udelay (200);
	memctl->memc_mcr = 0x80008130;	/* SDRAM bank 0 - execute twice */
	udelay (1);
	memctl->memc_mamr = (CFG_MAMR_8COL & ~(MAMR_TLFA_MSK)) | MAMR_TLFA_4X;
	udelay (200);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */
	udelay (1000);

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR_2BK_4K;	/* 16: but should be: CFG_MPTPR_1BK_4K */

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CFG_MAMR_8COL, (long *) SDRAM_BASE4_PRELIM,
			   SDRAM_MAX_SIZE);
	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CFG_MAMR_9COL, (long *) SDRAM_BASE4_PRELIM,
			   SDRAM_MAX_SIZE);

	if (size8 < size9) {	/* leave configuration at 9 columns     */
		size_b0 = size9;
/*	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
	} else {		/* back to 8 columns                    */
		size_b0 = size8;
		memctl->memc_mamr = CFG_MAMR_8COL;
		udelay (500);
/*	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
	}

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CFG_MPTPR_2BK_4K;
		udelay (1000);
	}

	/* SDRAM Bank 0 is bigger - map first       */

	memctl->memc_or4 = ((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
	memctl->memc_br4 = (CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	udelay (10000);

	return (size_b0);
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

void doc_init (void)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	upmconfig (UPMB, (uint *) static_table,
		   sizeof (static_table) / sizeof (uint));
	memctl->memc_mbmr = MAMR_DSA_1_CYCL;

	doc_probe (FLASH_BASE1_PRELIM);
}

/*
 * (C) Copyright 2000, 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * (C) Copyright 2001
 * Torsten Stevens, FHG IMS, stevens@ims.fhg.de
 * Bruno Achauer, Exet AG, bruno@exet-ag.de.
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
 * Derived from ../tqm8xx/tqm8xx.c
 */

#include <common.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1ff77c35, 0xefeabc34, 0x1fb57c35,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,	/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe1bbbc04, 0x1ff77c47,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, 0xfffffc07,	/* last */
	_NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7ffffc07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
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
	printf ("Board: Lantec special edition rev.%d\n", CONFIG_LANTEC);
	return 0;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0;
	int i;

	/*
	 * Configure UPMA for SDRAM
	 */
	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	memctl->memc_mptpr = CONFIG_SYS_MPTPR_1BK_8K /* XXX CONFIG_SYS_MPTPR XXX */ ;

	/* burst length=4, burst type=sequential, CAS latency=2 */
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 3 to the SDRAM bank at preliminary address.
	 */
	memctl->memc_or3 = CONFIG_SYS_OR3_PRELIM;
	memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM;

	/* initialize memory address register */
	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;	/* refresh not enabled yet */

	/* mode initialization (offset 5) */
	udelay (200);		/* 0x80006105 */
	memctl->memc_mcr =
		MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x05);

	/* run 2 refresh sequence with 4-beat refresh burst (offset 0x30) */
	udelay (1);		/* 0x80006130 */
	memctl->memc_mcr =
		MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x30);
	udelay (1);		/* 0x80006130 */
	memctl->memc_mcr =
		MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x30);

	udelay (1);		/* 0x80006106 */
	memctl->memc_mcr =
		MCR_OP_RUN | MCR_MB_CS3 | MCR_MLCF (1) | MCR_MAD (0x06);

	memctl->memc_mamr |= MAMR_PTAE;	/* refresh enabled */

	udelay (200);

	/* Need at least 10 DRAM accesses to stabilize */
	for (i = 0; i < 10; ++i) {
		volatile unsigned long *addr =
			(volatile unsigned long *) SDRAM_BASE3_PRELIM;
		unsigned long val;

		val = *(addr + i);
		*(addr + i) = val;
	}

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 */
	size_b0 = dram_size (CONFIG_SYS_MAMR_8COL,
			     (long *) SDRAM_BASE3_PRELIM, SDRAM_MAX_SIZE);

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL | MAMR_PTAE;

	/*
	 * Final mapping:
	 */

	memctl->memc_or3 = ((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
	memctl->memc_br3 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;
	udelay (1000);

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
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size (base, maxsize));
}

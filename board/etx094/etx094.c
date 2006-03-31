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

DECLARE_GLOBAL_DATA_PTR;

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);
static void read_hw_vers (void);

/* ------------------------------------------------------------------------- */

#define _NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {

	/* single read   (offset 0x00 in upm ram) */

	0xEECEFC24, 0x100DFC24, 0xE02FBC04, 0x01AA7C04,
	0x1FB5FC00, 0xFFFFFC05, _NOT_USED_, _NOT_USED_,

	/* burst read    (offset 0x08 in upm ram) */

	0xEECEFC24, 0x100DFC24, 0xE0FFBC04, 0x10FF7C04,
	0xF0FFFC00, 0xF0FFFC00, 0xF0FFFC00, 0xFFFFFC00,
	0xFFFFFC05, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* single write  (offset 0x18 in upm ram) */

	0xEECEFC24, 0x100DFC24, 0xE02BBC04, 0x01A27C00,
	0xEFAAFC04, 0x1FB5FC05, _NOT_USED_, _NOT_USED_,

	/* burst write   (offset 0x20 in upm ram) */

	0xEECEFC24, 0x103DFC24, 0xE0FBBC00, 0x10F77C00,
	0xF0FFFC00, 0xF0FFFC00, 0xF0FFFC04, 0xFFFFFC05,

	/* init part1      (offset 0x28 in upm ram) */

	0xEFFAFC3C, 0x1FF4FC34, 0xEFFCBC34, 0x1FFC3C34,
	0xFFFC3C35, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/* refresh   (offset 0x30 in upm ram) */

	0xEFFEBC0C, 0x1FFD7C04, 0xFFFFFC04, 0xFFFFFC05,

	/* init part2     (offset 0x34 in upm ram) */

	0xFFFEBC04, 0xEFFC3CB4, 0x1FFC3C34, 0xFFFC3C34,
	0xFFFC3C34, 0xEFE83CB4, 0x1FB57C35, _NOT_USED_,

	/* exception     (offset 0x3C in upm ram) */

	0xFFFFFC05, _NOT_USED_, _NOT_USED_, _NOT_USED_,

};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 * Test ETX ID string (ETX_xxx...)
 *
 * Return 1 always.
 */

int checkboard (void)
{
	char *s = getenv ("serial#");
	char *e;

	puts ("Board: ");

#ifdef SB_ETX094
	gd->board_type = 0; /* 0 = 2SDRAM-Device */
#else
	gd->board_type = 1; /* 1 = 1SDRAM-Device */
#endif

	if (!s || strncmp (s, "ETX_", 4)) {
		puts ("### No HW ID - assuming ETX_094\n");
		read_hw_vers ();
		return (0);
	}

	for (e = s; *e; ++e) {
		if (*e == ' ')
			break;
	}

	for (; s < e; ++s) {
		putc (*s);
	}
	putc ('\n');

	read_hw_vers ();
	return (0);
}

/* ------------------------------------------------------------------------- */

long int initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0, size_b1, size8, size9;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CFG_MPTPR_1BK_4K;	/* MPTPR_PTP_DIV32 0x0200 */

	/* A3(SDRAM)=0      => Bursttype = Sequential
	 * A2-A0(SDRAM)=010 => Burst length = 4
	 * A4-A6(SDRAM)=010 => CasLat=2
	 */
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 2 and 3 to the SDRAM banks 2 and 3 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or2 = CFG_OR2_PRELIM;
	memctl->memc_br2 = CFG_BR2_PRELIM;

	if (board_type == 0) {	/* "L" type boards have only one bank SDRAM */
		memctl->memc_or3 = CFG_OR3_PRELIM;
		memctl->memc_br3 = CFG_BR3_PRELIM;
	}

	memctl->memc_mamr = CFG_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80004128;	/* SDRAM bank 0 (CS2) - Init Part 1 */
	memctl->memc_mcr = 0x80004734;	/* SDRAM bank 0 (CS2) - Init Part 2 */
	udelay (1);

	if (board_type == 0) {		/* "L" type boards have only one bank SDRAM */
		memctl->memc_mcr = 0x80006128;	/* SDRAM bank 1 (CS3) - Init Part 1 */
		memctl->memc_mcr = 0x80006734;	/* SDRAM bank 1 (CS3) - Init Part 2 */
		udelay (1);
	}

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CFG_MAMR_8COL, (long *) SDRAM_BASE2_PRELIM,
					   SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CFG_MAMR_9COL, (long *) SDRAM_BASE2_PRELIM,
					   SDRAM_MAX_SIZE);

	if (size8 < size9) {		/* leave configuration at 9 columns */
		size_b0 = size9;
/*	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
	} else {					/* back to 8 columns            */
		size_b0 = size8;
		memctl->memc_mamr = CFG_MAMR_8COL;
		udelay (500);
/*	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
	}

	if (board_type == 0) {		/* "L" type boards have only one bank SDRAM */
		/*
		 * Check Bank 1 Memory Size
		 * use current column settings
		 * [9 column SDRAM may also be used in 8 column mode,
		 *  but then only half the real size will be used.]
		 */
		size_b1 =
				dram_size (memctl->memc_mamr, (long *) SDRAM_BASE3_PRELIM,
						   SDRAM_MAX_SIZE);
/*	debug ("SDRAM Bank 1: %ld MB\n", size8 >> 20);	*/
	} else {
		size_b1 = 0;
	}

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000) && (size_b1 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CFG_MPTPR_2BK_4K;	/*DIV16 */
		udelay (1000);
	}

	/*
	 * Final mapping: map bigger bank first
	 */
	if (size_b1 > size_b0) {	/* SDRAM Bank 1 is bigger - map first   */

		memctl->memc_or3 = ((-size_b1) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
		memctl->memc_br3 =
			(CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

		if (size_b0 > 0) {
			/*
			 * Position Bank 0 immediately above Bank 1
			 */
			memctl->memc_or2 =
				((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
			memctl->memc_br2 =
				((CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V)
				+ size_b1;
		} else {
			unsigned long reg;

			/*
			 * No bank 0
			 *
			 * invalidate bank
			 */
			memctl->memc_br2 = 0;

			/* adjust refresh rate depending on SDRAM type, one bank */
			reg = memctl->memc_mptpr;
			reg >>= 1;	/* reduce to CFG_MPTPR_1BK_8K / _4K */
			memctl->memc_mptpr = reg;
		}

	} else {			/* SDRAM Bank 0 is bigger - map first   */

		memctl->memc_or2 = ((-size_b0) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
		memctl->memc_br2 =
				(CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

		if (size_b1 > 0) {
			/*
			 * Position Bank 1 immediately above Bank 0
			 */
			memctl->memc_or3 =
					((-size_b1) & 0xFFFF0000) | CFG_OR_TIMING_SDRAM;
			memctl->memc_br3 =
					((CFG_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V)
					+ size_b0;
		} else {
			unsigned long reg;

			/*
			 * No bank 1
			 *
			 * invalidate bank
			 */
			memctl->memc_br3 = 0;

			/* adjust refresh rate depending on SDRAM type, one bank */
			reg = memctl->memc_mptpr;
			reg >>= 1;	/* reduce to CFG_MPTPR_1BK_8K / _4K */
			memctl->memc_mptpr = reg;
		}
	}

	udelay (10000);

	return (size_b0 + size_b1);
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

	return (get_ram_size(base, maxsize));
}

/* ------------------------------------------------------------------------- */

/* HW-ID Table (Bits: 2^9;2^7;2^5) */
#define HW_ID_0 0x0000
#define HW_ID_1 0x0020
#define HW_ID_2 0x0080
#define HW_ID_3 0x00a0
#define HW_ID_4 0x0200
#define HW_ID_5 0x0220
#define HW_ID_6 0x0280
#define HW_ID_7 0x02a0

void read_hw_vers ()
{
	unsigned short rd_msk = 0x02A0;

	/* HW-ID pin-definition */
	volatile immap_t *immr = (immap_t *) CFG_IMMR;

	immr->im_ioport.iop_pddir &= ~(rd_msk);
	immr->im_ioport.iop_pdpar &= ~(rd_msk);

	/* debug printf("State of PD: %x\n",immr->im_ioport.iop_pddat); */

	/* Check the HW-ID */
	printf ("HW-Version: ");
	switch (immr->im_ioport.iop_pddat & rd_msk) {
	case HW_ID_0:
		printf ("V0.1 - V0.3 / W97238-Q3162-A1-1-2\n");
		break;
	case HW_ID_1:
		printf ("V0.9 / W50037-Q1-D6-1\n");
		break;
	case HW_ID_2:
		printf ("NOT USED - assuming ID#2\n");
		break;
	case HW_ID_3:
		printf ("NOT USED - assuming ID#3\n");
		break;
	case HW_ID_4:
		printf ("NOT USED - assuming ID#4\n");
		break;
	case HW_ID_5:
		printf ("NOT USED - assuming ID#5\n");
		break;
	case HW_ID_6:
		printf ("NOT USED - assuming ID#6\n");
		break;
	case HW_ID_7:
		printf ("NOT USED - assuming ID#7\n");
		break;
	default:
		printf ("###Error###\n");
		break;
	}
}

/* ------------------------------------------------------------------------- */

/*
**=====================================================================
**
** Copyright (C) 2000, 2001, 2002, 2003
** The LEOX team <team@leox.org>, http://www.leox.org
**
** LEOX.org is about the development of free hardware and software resources
**   for system on chip.
**
** Description: U-Boot port on the LEOX's ELPT860 CPU board
** ~~~~~~~~~~~
**
**=====================================================================
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation; either version 2 of
** the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307 USA
**
**=====================================================================
*/

/*
** Note 1: In this file, you have to provide the following functions:
** ------
**              int             board_early_init_f(void)
**              int             checkboard(void)
**              phys_size_t     initdram(int board_type)
** called from 'board_init_f()' into 'common/board.c'
**
**              void            reset_phy(void)
** called from 'board_init_r()' into 'common/board.c'
*/

#include <common.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFFFFF

const uint init_sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x0FFCCC04, 0xFFFFFC04, 0x0FFC3C04, 0xFFFFFC04,
	0xFFFFFC04,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0xFFFFFC04, 0xFFFFFC04, 0x0FFC3C04,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0x0FFC3C04, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC04, 0x0FFC3C04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC04, 0xFFFFFC04, 0x0FFC3C04, 0xFFFFFC04,	/* last */
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04, 0x0FFC3C04,
	0xFFFFFC04, 0xFFFFFC04, 0x0FFFFC04, 0xFFFFFC04,	/* last */
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x0FFC3C04, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC04, 0x0FFC3C04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC34, 0x0FAC0C34,
	0xFFFFFC05, 0xFFFFFC04, 0x0FFCFC04, 0xFFFFFC05,	/* last */
};

const uint sdram_table[] = {
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x0F0FFC24, 0x0F0CFC04, 0xFF0FFC04, 0x00AF3C04,
	0xFF0FFC00,		/* last */
	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x0FFCCC04, 0xFFAFFC05, 0xFFAFFC05,	/* last */
	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x0F0FFC24, 0x0F0CFC04, 0xFF0FFC04, 0x00AF3C04,
	0xF00FFC00, 0xF00FFC00, 0xF00FFC00, 0xFF0FFC00,
	0x0FFCCC04, 0xFFAFFC05, 0xFFAFFC04, 0xFFAFFC04,
	0xFFAFFC04, 0xFFAFFC04, 0xFFAFFC04, 0xFFAFFC04,	/* last */
	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x0F0FFC24, 0x0F0CFC04, 0xFF0FFC04, 0x00AF0C00,
	0xFF0FFC04, 0x0FFCCC04, 0xFFAFFC05,	/* last */
	_NOT_USED_,
	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x0F0FFC24, 0x0F0CFC04, 0xFF0FFC00, 0x00AF0C00,
	0xF00FFC00, 0xF00FFC00, 0xF00FFC04, 0x0FFCCC04,
	0xFFAFFC04, 0xFFAFFC05, 0xFFAFFC04, 0xFFAFFC04,
	0xFFAFFC04, 0xFFAFFC04, 0xFFAFFC04, 0xFFAFFC04,	/* last */
	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x0FFC3C04, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC05, 0xFFFFFC04, 0xFFFFFC05, _NOT_USED_,
	0xFFAFFC04, 0xFFAFFC04, 0xFFAFFC04, 0xFFAFFC04,	/* last */
	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x0FFFFC34, 0x0FAC0C34, 0xFFFFFC05, 0xFFAFFC04,	/* last */
};

/* ------------------------------------------------------------------------- */

#define CONFIG_SYS_PC4    0x0800

#define CONFIG_SYS_DS1    CONFIG_SYS_PC4

/*
 * Very early board init code (fpga boot, etc.)
 */
int board_early_init_f (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	/*
	 * Light up the red led on ELPT860 pcb (DS1) (PCDAT)
	 */
	immr->im_ioport.iop_pcdat &= ~CONFIG_SYS_DS1;	/* PCDAT (DS1 = 0)                */
	immr->im_ioport.iop_pcpar &= ~CONFIG_SYS_DS1;	/* PCPAR (0=general purpose I/O)  */
	immr->im_ioport.iop_pcdir |= CONFIG_SYS_DS1;	/* PCDIR (I/O: 0=input, 1=output) */

	return (0);		/* success */
}

/*
 * Check Board Identity:
 *
 * Test ELPT860 ID string
 *
 * Return 1 if no second DRAM bank, otherwise returns 0
 */

int checkboard (void)
{
	char *s = getenv ("serial#");

	if (!s || strncmp (s, "ELPT860", 7))
		printf ("### No HW ID - assuming ELPT860\n");

	return (0);		/* success */
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size8, size9;
	long int size_b0 = 0;

	/*
	 * This sequence initializes SDRAM chips on ELPT860 board
	 */
	upmconfig (UPMA, (uint *) init_sdram_table,
		   sizeof (init_sdram_table) / sizeof (uint));

	memctl->memc_mptpr = 0x0200;
	memctl->memc_mamr = 0x18002111;

	memctl->memc_mar = 0x00000088;
	memctl->memc_mcr = 0x80002000;	/* CS1: SDRAM bank 0 */

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

	/*
	 * The following value is used as an address (i.e. opcode) for
	 * the LOAD MODE REGISTER COMMAND during SDRAM initialisation. If
	 * the port size is 32bit the SDRAM does NOT "see" the lower two
	 * address lines, i.e. mar=0x00000088 -> opcode=0x00000022 for
	 * MICRON SDRAMs:
	 * ->    0 00 010 0 010
	 *       |  |   | |   +- Burst Length = 4
	 *       |  |   | +----- Burst Type   = Sequential
	 *       |  |   +------- CAS Latency  = 2
	 *       |  +----------- Operating Mode = Standard
	 *       +-------------- Write Burst Mode = Programmed Burst Length
	 */
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller banks 2 and 3 to the SDRAM banks 2 and 3 at
	 * preliminary addresses - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80002105;	/* CS1: SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80002230;	/* CS1: SDRAM bank 0 - execute twice */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL,
			   SDRAM_BASE1_PRELIM, SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL,
			   SDRAM_BASE1_PRELIM, SDRAM_MAX_SIZE);

	if (size8 < size9) {	/* leave configuration at 9 columns       */
		size_b0 = size9;
		/* debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20); */
	} else {		/* back to 8 columns                      */

		size_b0 = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
		/* debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20); */
	}

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if (size_b0 < 0x02000000) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping: map bigger bank first
	 */
	memctl->memc_or1 = ((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
	memctl->memc_br1 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	{
		unsigned long reg;

		/* adjust refresh rate depending on SDRAM type, one bank */
		reg = memctl->memc_mptpr;
		reg >>= 1;	/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
		memctl->memc_mptpr = reg;
	}

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

static long int
dram_size (long int mamr_value, long int *base, long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size (base, maxsize));
}

/* ------------------------------------------------------------------------- */

#define CONFIG_SYS_PA1     0x4000
#define CONFIG_SYS_PA2     0x2000

#define CONFIG_SYS_LBKs    (CONFIG_SYS_PA2 | CONFIG_SYS_PA1)

void reset_phy (void)
{
	volatile immap_t *immr = (immap_t *) CONFIG_SYS_IMMR;

	/*
	 * Ensure LBK LXT901 ethernet 1 & 2 = 0 ... for normal loopback in effect
	 *                                          and no AUI loopback
	 */
	immr->im_ioport.iop_padat &= ~CONFIG_SYS_LBKs;	/* PADAT (LBK eth 1&2 = 0)        */
	immr->im_ioport.iop_papar &= ~CONFIG_SYS_LBKs;	/* PAPAR (0=general purpose I/O)  */
	immr->im_ioport.iop_padir |= CONFIG_SYS_LBKs;	/* PADIR (I/O: 0=input, 1=output) */
}

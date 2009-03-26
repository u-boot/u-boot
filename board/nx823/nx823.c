/*
 * (C) Copyright 2001
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
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
#include <malloc.h>
#include <mpc8xx.h>
#include <net.h>

DECLARE_GLOBAL_DATA_PTR;

static long int dram_size (long int, long int *, long int);

#define	_NOT_USED_	0xFFFFFFFF

const uint sdram_table[] = {
#if (MPC8XX_SPEED <= 50000000L)
	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x0F07EC04, 0x01BBD804, 0x1FF7F440, 0xFFFFFC07,
	0xFFFFFFFF,

	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1FE7F434, 0xEFABE834, 0x1FA7D435,

	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x0F07EC04, 0x10EFDC04, 0xF0AFFC00, 0xF0AFFC00,
	0xF1AFFC00, 0xFFAFFC40, 0xFFAFFC07, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x0E07E804, 0x01BBD000, 0x1FF7F447, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x0E07E800, 0x10EFD400, 0xF0AFFC00, 0xF0AFFC00,
	0xF1AFFC47, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FF7DC84, 0xFFFFFC04, 0xFFFFFC84, 0xFFFFFC07,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,

	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF
#else

	/*
	 * Single Read. (Offset 0 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAFEC04, 0x11AFDC04, 0xEFBBF800,
	0x1FF7F447,

	/*
	 * SDRAM Initialization (offset 5 in UPMA RAM)
	 *
	 * This is no UPM entry point. The following definition uses
	 * the remaining space to establish an initialization
	 * sequence, which is executed by a RUN command.
	 *
	 */
	0x1FF7F434, 0xEFEBE834, 0x1FB7D435,

	/*
	 * Burst Read. (Offset 8 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAFEC04, 0x10AFDC04, 0xF0AFFC00,
	0xF0AFFC00, 0xF1AFFC00, 0xEFBBF800, 0x1FF7F447,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAFE800, 0x01BBD004, 0x1FF7F447,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Burst Write. (Offset 20 in UPMA RAM)
	 */
	0x1F07FC04, 0xEEAFE800, 0x10AFD400, 0xF0AFFC00,
	0xF0AFFC00, 0xE1BBF804, 0x1FF7F447, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Refresh  (Offset 30 in UPMA RAM)
	 */
	0x1FF7DC84, 0xFFFFFC04, 0xFFFFFC04, 0xFFFFFC04,
	0xFFFFFC84, 0xFFFFFC07,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_,

	/*
	 * Exception. (Offset 3c in UPMA RAM)
	 */
	0x7FFFFC07,		/* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_,
#endif
};

/* ------------------------------------------------------------------------- */


/*
 * Check Board Identity:
 *
 */

int checkboard (void)
{
	printf ("Board: Nexus NX823");
	return (0);
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size_b0, size_b1, size8, size9;

	upmconfig (UPMA, (uint *) sdram_table,
		   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Up to 2 Banks of 64Mbit x 2 devices
	 * Initial builds only have 1
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_1BK_4K;
	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller SDRAM bank 0
	 */
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;
	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */
	udelay (200);

	/*
	 * Map controller SDRAM bank 1
	 */
	memctl->memc_or2 = CONFIG_SYS_OR2_PRELIM;
	memctl->memc_br2 = CONFIG_SYS_BR2_PRELIM;

	/*
	 * Perform SDRAM initializsation sequence
	 */
	memctl->memc_mcr = 0x80002105;	/* SDRAM bank 0 */
	udelay (1);
	memctl->memc_mcr = 0x80002230;	/* SDRAM bank 0 - execute twice */
	udelay (1);

	memctl->memc_mcr = 0x80004105;	/* SDRAM bank 1 */
	udelay (1);
	memctl->memc_mcr = 0x80004230;	/* SDRAM bank 1 - execute twice */
	udelay (1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */
	udelay (1000);

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

	memctl->memc_mar = 0x00000088;


	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL, (long *) SDRAM_BASE1_PRELIM,
			   SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL, (long *) SDRAM_BASE1_PRELIM,
			   SDRAM_MAX_SIZE);

	if (size8 < size9) {	/* leave configuration at 9 columns     */
		size_b0 = size9;
/*	debug ("SDRAM Bank 0 in 9 column mode: %ld MB\n", size >> 20);	*/
	} else {		/* back to 8 columns                    */
		size_b0 = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
/*	debug ("SDRAM Bank 0 in 8 column mode: %ld MB\n", size >> 20);	*/
	}

	/*
	 * Check Bank 1 Memory Size
	 * use current column settings
	 * [9 column SDRAM may also be used in 8 column mode,
	 *  but then only half the real size will be used.]
	 */
	size_b1 = dram_size (memctl->memc_mamr, (long *) SDRAM_BASE2_PRELIM,
			     SDRAM_MAX_SIZE);
/*	debug ("SDRAM Bank 1: %ld MB\n", size8 >> 20);	*/

	udelay (1000);

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000) && (size_b1 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping: map bigger bank first
	 */
	if (size_b1 > size_b0) {	/* SDRAM Bank 1 is bigger - map first   */

		memctl->memc_or2 =
			((-size_b1) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
		memctl->memc_br2 =
			(CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

		if (size_b0 > 0) {
			/*
			 * Position Bank 0 immediately above Bank 1
			 */
			memctl->memc_or1 =
				((-size_b0) & 0xFFFF0000) |
				CONFIG_SYS_OR_TIMING_SDRAM;
			memctl->memc_br1 =
				((CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA |
				 BR_V)
				+ size_b1;
		} else {
			unsigned long reg;

			/*
			 * No bank 0
			 *
			 * invalidate bank
			 */
			memctl->memc_br1 = 0;

			/* adjust refresh rate depending on SDRAM type, one bank */
			reg = memctl->memc_mptpr;
			reg >>= 1;	/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
			memctl->memc_mptpr = reg;
		}

	} else {		/* SDRAM Bank 0 is bigger - map first   */

		memctl->memc_or1 =
			((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
		memctl->memc_br1 =
			(CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

		if (size_b1 > 0) {
			/*
			 * Position Bank 1 immediately above Bank 0
			 */
			memctl->memc_or2 =
				((-size_b1) & 0xFFFF0000) |
				CONFIG_SYS_OR_TIMING_SDRAM;
			memctl->memc_br2 =
				((CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA |
				 BR_V)
				+ size_b0;
		} else {
			unsigned long reg;

			/*
			 * No bank 1
			 *
			 * invalidate bank
			 */
			memctl->memc_br2 = 0;

			/* adjust refresh rate depending on SDRAM type, one bank */
			reg = memctl->memc_mptpr;
			reg >>= 1;	/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
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
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size (base, maxsize));
}

int misc_init_r (void)
{
	int i;
	char tmp[50];
	uchar ethaddr[6];
	bd_t *bd = gd->bd;
	ulong *my_sernum = (unsigned long *)&bd->bi_sernum;

	/* load unique serial number */
	for (i = 0; i < 8; ++i)
		bd->bi_sernum[i] = *(u_char *) (CONFIG_SYS_FLASH_SN_BASE + i);

	/* save env variables according to sernum */
	sprintf (tmp, "%08lx%08lx", my_sernum[0], my_sernum[1]);
	setenv ("serial#", tmp);

	if (!eth_getenv_enetaddr("ethaddr", ethaddr)) {
		ethaddr[0] = 0x10;
		ethaddr[1] = 0x20;
		ethaddr[2] = 0x30;
		ethaddr[3] = bd->bi_sernum[1] << 4 | bd->bi_sernum[2];
		ethaddr[4] = bd->bi_sernum[5];
		ethaddr[5] = bd->bi_sernum[6];
	}

	return 0;
}

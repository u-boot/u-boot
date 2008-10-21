/*
 * (C) Copyright 2006 Detlev Zundel, dzu@denx.de
 * (C) Copyright 2001
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
#include <config.h>
#include <mpc8xx.h>

/*
 *  Memory Controller Using
 *
 *  CS0 - Flash memory            (0x40000000)
 *  CS3 - SDRAM                   (0x00000000}
 */

/* ------------------------------------------------------------------------- */

#define _not_used_	0xffffffff

const uint sdram_table[] = {
	/* single read. (offset 0 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,

	/* MRS initialization (offset 5) */

	0x1ff77c34, 0xefeabc34, 0x1fb57c35,

	/* burst read. (offset 8 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* single write. (offset 18 in upm RAM) */
	0x1f27fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* burst write. (offset 20 in upm RAM) */
	0x1f07fc04, 0xeeaebc00, 0x10ad7c00, 0xf0affc00,
	0xf0affc00, 0xe1bbbc04, 0x1ff77c47, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* refresh. (offset 30 in upm RAM) */
	0x1ff5fc84, 0xfffffc04, 0xfffffc04, 0xfffffc04,
	0xfffffc84, 0xfffffc07, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* exception. (offset 3c in upm RAM) */
	0x7ffffc07, _not_used_, _not_used_, _not_used_
};

const uint nand_flash_table[] = {
	/* single read. (offset 0 in upm RAM) */
	0x0ff3fc04, 0x0ff3fc04, 0x0ff3fc04, 0x0ffffc04,
	0xfffffc00, 0xfffffc05, 0xfffffc05, 0xfffffc05,

	/* burst read. (offset 8 in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,

	/* single write. (offset 18 in upm RAM) */
	0x00fffc04, 0x00fffc04, 0x00fffc04, 0x0ffffc04,
	0x0ffffc84, 0x0ffffc84, 0xfffffc00, 0xfffffc05,

	/* burst write. (offset 20 in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,

	/* refresh. (offset 30 in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05,

	/* exception. (offset 3c in upm RAM) */
	0xffffcc05, 0xffffcc05, 0xffffcc05, 0xffffcc05
};

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 */

int checkboard (void)
{
#if !defined(CONFIG_CP850)
	puts ("Board: NC650");
#else
	puts ("Board: CP850");
#endif
#if defined(CONFIG_IDS852_REV1)
	puts (" with IDS852 rev 1 module\n");
#elif defined(CONFIG_IDS852_REV2)
	puts (" with IDS852 rev 2 module\n");
#endif
	return 0;
}

/* ------------------------------------------------------------------------- */

static long int dram_size (long int, long int *, long int);

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size8, size9;
	long int size_b0 = 0;
	unsigned long reg;

	upmconfig (UPMA, (uint *) sdram_table,
			   sizeof (sdram_table) / sizeof (uint));

	/*
	 * Preliminary prescaler for refresh (depends on number of
	 * banks): This value is selected for four cycles every 62.4 us
	 * with two SDRAM banks or four cycles every 31.2 us with one
	 * bank. It will be adjusted after memory sizing.
	 */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_8K;

	memctl->memc_mar = 0x00000088;

	/*
	 * Map controller bank 1 to the SDRAM bank at
	 * preliminary address - these have to be modified after the
	 * SDRAM size has been determined.
	 */
	memctl->memc_or3 = CONFIG_SYS_OR3_PRELIM;
	memctl->memc_br3 = CONFIG_SYS_BR3_PRELIM;

	memctl->memc_mamr = CONFIG_SYS_MAMR_8COL & (~(MAMR_PTAE));	/* no refresh yet */

	udelay (200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr = 0x80006105;	/* SDRAM bank 0 */
	udelay (200);
	memctl->memc_mcr = 0x80006230;	/* SDRAM bank 0 - execute twice */
	udelay (200);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	udelay (1000);

	/*
	 * Check Bank 0 Memory Size for re-configuration
	 *
	 * try 8 column mode
	 */
	size8 = dram_size (CONFIG_SYS_MAMR_8COL, SDRAM_BASE3_PRELIM, SDRAM_MAX_SIZE);

	udelay (1000);

	/*
	 * try 9 column mode
	 */
	size9 = dram_size (CONFIG_SYS_MAMR_9COL, SDRAM_BASE3_PRELIM, SDRAM_MAX_SIZE);

	udelay (1000);

	if (size8 < size9) {
		size_b0 = size9;
	} else {
		size_b0 = size8;
		memctl->memc_mamr = CONFIG_SYS_MAMR_8COL;
		udelay (500);
	}

	/*
	 * Adjust refresh rate depending on SDRAM type, both banks.
	 * For types > 128 MBit leave it at the current (fast) rate
	 */
	if ((size_b0 < 0x02000000)) {
		/* reduce to 15.6 us (62.4 us / quad) */
		memctl->memc_mptpr = CONFIG_SYS_MPTPR_2BK_4K;
		udelay (1000);
	}

	/*
	 * Final mapping
	 */

	memctl->memc_or3 = ((-size_b0) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;
	memctl->memc_br3 = (CONFIG_SYS_SDRAM_BASE & BR_BA_MSK) | BR_MS_UPMA | BR_V;

	/* adjust refresh rate depending on SDRAM type, one bank */
	reg = memctl->memc_mptpr;
	reg >>= 1;					/* reduce to CONFIG_SYS_MPTPR_1BK_8K / _4K */
	memctl->memc_mptpr = reg;

	udelay (10000);

	/* Configure UPMB for NAND flash access */
	upmconfig (UPMB, (uint *) nand_flash_table,
			   sizeof (nand_flash_table) / sizeof (uint));

	memctl->memc_mbmr = CONFIG_SYS_MBMR_NAND;

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

static long int dram_size (long int mamr_value, long int *base, long int maxsize)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;

	memctl->memc_mamr = mamr_value;

	return (get_ram_size(base, maxsize));
}


#if defined(CONFIG_CP850)

#define DPRAM_VARNAME           "KP850DIP"
#define PARAM_ADDR              0x7C0
#define NAME_ADDR               0x7F8
#define BOARD_NAME              "KP01"
#define DEFAULT_LB              "241111"

int misc_init_r(void)
{
	int             iCompatMode = 0;
	char            *pParam = NULL;
	char            *envlb;

	/*
	   First byte in CPLD read address space signals compatibility mode
	   0 - cp850
	   1 - kp852
	*/
	pParam = (char*)(CONFIG_SYS_CPLD_BASE);
	if( *pParam != 0)
		iCompatMode = 1;

	if ( iCompatMode != 0) {
		/*
		   In KP852 compatibility mode we have to write to
		   DPRAM as early as possible the binary coded
		   line config and board name.
		   The line config is derived from the environment
		   variable DPRAM_VARNAME by converting from ASCII
		   to binary per character.
		*/
		if ( (envlb = getenv ( DPRAM_VARNAME )) == 0) {
			setenv( DPRAM_VARNAME, DEFAULT_LB);
			envlb = DEFAULT_LB;
		}

		/* Status string */
		printf("Mode:  KP852(LB=%s)\n", envlb);

		/* copy appl init */
		pParam = (char*)(DPRAM_BASE_ADDR + PARAM_ADDR);
		while (*envlb) {
			*(pParam++) = *(envlb++) - '0';
		}
		*pParam = '\0';

		/* copy board id */
		pParam = (char*)(DPRAM_BASE_ADDR + NAME_ADDR);
		strcpy( pParam, BOARD_NAME);
	} else {
		puts("Mode:  CP850\n");
	}

	return 0;
}
#endif

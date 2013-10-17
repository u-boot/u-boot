/*
 * (C) Copyright 2004
 * Sam Song, IEMC. SHU, samsongshu@yahoo.com.cn
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Sam Song
 * U-Boot port on RPXlite DW board : RPXlite_DW or LITE_DW
 * Tested on working at 64MHz(CPU)/32MHz(BUS),48MHz/24MHz
 * with 64MB, 2 SDRAM Micron chips,MT48LC16M16A2-75.
 */

#include <common.h>
#include <mpc8xx.h>

/* ------------------------------------------------------------------------- */
static long int dram_size (long int, long int *, long int);
/* ------------------------------------------------------------------------- */

#define	_NOT_USED_	0xFFFFCC25

const uint sdram_table[] =
{
	/*
	 * Single Read. (Offset 00h in UPMA RAM)
	 */
	0x0F03CC04, 0x00ACCC24, 0x1FF74C20, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_,

	/*
	 * Burst Read. (Offset 08h in UPMA RAM)
	 */
	0x0F03CC04, 0x00ACCC24, 0x00FFCC20, 0x00FFCC20,
	0x01FFCC20, 0x1FF74C20, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/*
	 * Single Write. (Offset 18h in UPMA RAM)
	 */
	0x0F03CC02, 0x00AC0C24, 0x1FF74C25, /* last */
	_NOT_USED_, _NOT_USED_, 0x0FA00C34,0x0FFFCC35,
	_NOT_USED_,

	/*
	 * Burst Write. (Offset 20h in UPMA RAM)
	 */
	0x0F03CC00, 0x00AC0C20, 0x00FFFC20, 0x00FFFC22,
	0x01FFFC24, 0x1FF74C25, /* last */
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_,

	/*
	 * Refresh. (Offset 30h in UPMA RAM)
	 */
	0x0FF0CC24, 0xFFFFCC24, _NOT_USED_, _NOT_USED_,
	_NOT_USED_, _NOT_USED_, 0xEFFB8C34, 0x0FF74C34,
	0x0FFACCB4, 0x0FF5CC34, 0x0FFFCC34, 0x0FFFCCB4,
	/* INIT sequence RAM WORDS
	 * SDRAM Initialization (offset 0x36 in UPMA RAM)
	 * The above definition uses the remaining space
	 * to establish an initialization sequence,
	 * which is executed by a RUN command.
	 * The sequence is COMMAND INHIBIT(NOP),Precharge,
	 * Load Mode Register,NOP,Auto Refresh.
	 */

	/*
	 * Exception. (Offset 3Ch in UPMA RAM)
	 */
	0x0FEA8C34, 0x1FB54C34, 0xFFFFCC34, _NOT_USED_
};

/*
 * Check Board Identity:
 */

int checkboard (void)
{
	puts ("Board: RPXlite_DW\n") ;
	return (0) ;
}

/* ------------------------------------------------------------------------- */

phys_size_t initdram (int board_type)
{
	volatile immap_t     *immap  = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	long int size9;

	upmconfig(UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	/* Refresh clock prescalar */
	memctl->memc_mptpr = CONFIG_SYS_MPTPR ;

	memctl->memc_mar  = 0x00000088;

	/* Map controller banks 1 to the SDRAM bank */
	memctl->memc_or1 = CONFIG_SYS_OR1_PRELIM;
	memctl->memc_br1 = CONFIG_SYS_BR1_PRELIM;

	memctl->memc_mamr = CONFIG_SYS_MAMR_9COL & (~(MAMR_PTAE)); /* no refresh yet */
	/*Disable Periodic timer A. */

	udelay(200);

	/* perform SDRAM initializsation sequence */

	memctl->memc_mcr  = 0x80002236; /* SDRAM bank 0 - refresh twice */

	udelay(1);

	memctl->memc_mamr |= MAMR_PTAE;	/* enable refresh */

	/*Enable Periodic timer A */

	udelay (1000);

	 /* Check Bank 0 Memory Size
	  * try 9 column mode
	  */

	size9 = dram_size (CONFIG_SYS_MAMR_9COL, SDRAM_BASE_PRELIM, SDRAM_MAX_SIZE);

	/*
	 * Final mapping:
	 */

	memctl->memc_or1 = ((-size9) & 0xFFFF0000) | CONFIG_SYS_OR_TIMING_SDRAM;

	udelay (1000);

	return (size9);
}

void rpxlite_init (void)
{
	/* Enable NVRAM */
	*((uchar *) BCSR0) |= BCSR0_ENNVRAM;
}

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

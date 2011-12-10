/*
 * (C) Copyright 2001, 2002
 * Dave Ellis, SIXNET, dge@sixnetio.com.
 *  Based on code by:
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * and other contributors to U-Boot. See file CREDITS for list
 * of people who contributed to this  project.
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
#include <jffs2/jffs2.h>
#include <mpc8xx.h>
#include <net.h>	/* for eth_init() */
#include <rtc.h>
#include "sixnet.h"
#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

#define ORMASK(size) ((-size) & OR_AM_MSK)

static long ram_size(ulong *, long);

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress (int status)
{
#if defined(CONFIG_STATUS_LED)
# if defined(STATUS_LED_BOOT)
	if (status == BOOTSTAGE_ID_RUN_OS) {
		/* ready to transfer to kernel, make sure LED is proper state */
		status_led_set(STATUS_LED_BOOT, CONFIG_BOOT_LED_STATE);
	}
# endif /* STATUS_LED_BOOT */
#endif /* CONFIG_STATUS_LED */
}
#endif

/* ------------------------------------------------------------------------- */

/*
 * Check Board Identity:
 * returns 0 if recognized, -1 if unknown
 */

int checkboard (void)
{
	puts ("Board: SIXNET SXNI855T\n");
	return 0;
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_CMD_PCMCIA)
#error "SXNI855T has no PCMCIA port"
#endif

/* ------------------------------------------------------------------------- */

#define _not_used_ 0xffffffff

/* UPMB table for dual UART. */

/* this table is for 50MHz operation, it should work at all lower speeds */
const uint duart_table[] =
{
	/* single read. (offset 0 in upm RAM) */
	0xfffffc04, 0x0ffffc04, 0x0ff3fc04, 0x0ff3fc04,
	0x0ff3fc00, 0x0ff3fc04, 0xfffffc04, 0xfffffc05,

	/* burst read. (offset 8 in upm RAM) */
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* single write. (offset 18 in upm RAM) */
	0xfffffc04, 0x0ffffc04, 0x00fffc04, 0x00fffc04,
	0x00fffc04, 0x00fffc00, 0xfffffc04, 0xfffffc05,

	/* burst write. (offset 20 in upm RAM) */
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* refresh. (offset 30 in upm RAM) */
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* exception. (offset 3c in upm RAM) */
	_not_used_, _not_used_, _not_used_, _not_used_,
};

/* Load FPGA very early in boot sequence, since it must be
 * loaded before the 16C2550 serial channels can be used as
 * console channels.
 *
 * Note: Much of the configuration is not complete. The
 *       stack is in DPRAM since SDRAM has not been initialized,
 *       so the stack must be kept small. Global variables
 *       are still in FLASH, so they cannot be written.
 *	 Only the FLASH, DPRAM, immap and FPGA can be addressed,
 *       the other chip selects may not have been initialized.
 *       The clocks have been initialized, so udelay() can be
 *       used.
 */
#define FPGA_DONE	0x0080	/* PA8, input, high when FPGA load complete */
#define FPGA_PROGRAM_L	0x0040	/* PA9, output, low to reset, high to start */
#define FPGA_INIT_L	0x0020	/* PA10, input, low indicates not ready	*/
#define fpga (*(volatile unsigned char *)(CONFIG_SYS_FPGA_PROG))	/* FPGA port */

int board_postclk_init (void)
{

	/* the data to load to the XCSxxXL FPGA */
	static const unsigned char fpgadata[] = {
# include "fpgadata.c"
	};

	volatile immap_t     *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
#define porta (immap->im_ioport.iop_padat)
	const unsigned char* pdata;

	/* /INITFPGA and DONEFPGA signals are inputs */
	immap->im_ioport.iop_padir &= ~(FPGA_INIT_L | FPGA_DONE);

	/* Force output pin to begin at 0, /PROGRAM asserted (0) resets FPGA */
	porta &= ~FPGA_PROGRAM_L;

	/* Set FPGA as an output */
	immap->im_ioport.iop_padir |= FPGA_PROGRAM_L;

	/* delay a little to make sure FPGA sees it, really
	 * only need less than a microsecond.
	 */
	udelay(10);

	/* unassert /PROGRAM */
	porta |= FPGA_PROGRAM_L;

	/* delay while FPGA does last erase, indicated by
	 * /INITFPGA going high. This should happen within a
	 * few milliseconds.
	 */
	/* ### FIXME - a timeout check would be good, maybe flash
	 * the status LED to indicate the error?
	 */
	while ((porta & FPGA_INIT_L) == 0)
		; /* waiting */

	/* write program data to FPGA at the programming address
	 * so extra /CS1 strobes at end of configuration don't actually
	 * write to any registers.
	 */
	fpga = 0xff;		/* first write is ignored	*/
	fpga = 0xff;		/* fill byte			*/
	fpga = 0xff;		/* fill byte			*/
	fpga = 0x4f;		/* preamble code		*/
	fpga = 0x80; fpga = 0xaf; fpga = 0x9b; /* length (ignored) */
	fpga = 0x4b;		/* field check code */

	pdata = fpgadata;
	/* while no error write out each of the 28 byte frames */
	while ((porta & (FPGA_INIT_L | FPGA_DONE)) == FPGA_INIT_L
	       && pdata < fpgadata + sizeof(fpgadata)) {

		fpga = 0x4f;	/* preamble code */

		/* 21 bytes of data in a frame */
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++); fpga = *(pdata++);
		fpga = *(pdata++);

		fpga = 0x4b;	/* field check code		*/
		fpga = 0xff;	/* extended write cycle		*/
		fpga = 0x4b;	/* extended write cycle
				 * (actually 0x4b from bitgen.exe)
				 */
		fpga = 0xff;	/* extended write cycle		*/
		fpga = 0xff;	/* extended write cycle		*/
		fpga = 0xff;	/* extended write cycle		*/
	}

	fpga = 0xff;		/* startup byte			*/
	fpga = 0xff;		/* startup byte			*/
	fpga = 0xff;		/* startup byte			*/
	fpga = 0xff;		/* startup byte			*/

#if 0 /* ### FIXME */
	/* If didn't load all the data or FPGA_DONE is low the load failed.
	 * Maybe someday stop here and flash the status LED? The console
	 * is not configured, so can't print an error message. Can't write
	 * global variables to set a flag (except gd?).
	 * For now it must work.
	 */
#endif

	/* Now that the FPGA is loaded, set up the Dual UART chip
	 * selects. Must be done here since it may be used as the console.
	 */
	upmconfig(UPMB, (uint *)duart_table, sizeof(duart_table)/sizeof(uint));

	memctl->memc_mbmr = DUART_MBMR;
	memctl->memc_or5 = DUART_OR_VALUE;
	memctl->memc_br5 = DUART_BR5_VALUE;
	memctl->memc_or6 = DUART_OR_VALUE;
	memctl->memc_br6 = DUART_BR6_VALUE;

	return (0);
}

/* ------------------------------------------------------------------------- */

/* base address for SRAM, assume 32-bit port,  valid */
#define NVRAM_BR_VALUE   (CONFIG_SYS_SRAM_BASE | BR_PS_32 | BR_V)

/*  up to 64MB - will be adjusted for actual size */
#define NVRAM_OR_PRELIM  (ORMASK(CONFIG_SYS_SRAM_SIZE) \
	| OR_CSNT_SAM | OR_ACS_DIV4 | OR_BI | OR_SCY_5_CLK | OR_EHTR)
/*
 * Miscellaneous platform dependent initializations after running in RAM.
 */

int misc_init_r (void)
{
	volatile immap_t     *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	bd_t *bd = gd->bd;
	uchar enetaddr[6];

	memctl->memc_or2 = NVRAM_OR_PRELIM;
	memctl->memc_br2 = NVRAM_BR_VALUE;

	/* Is there any SRAM? Is it 16 or 32 bits wide? */

	/* First look for 32-bit SRAM */
	bd->bi_sramsize = ram_size((ulong*)CONFIG_SYS_SRAM_BASE, CONFIG_SYS_SRAM_SIZE);

	if (bd->bi_sramsize == 0) {
	    /* no 32-bit SRAM, but there could be 16-bit SRAM since
	     * it would report size 0 when configured for 32-bit bus.
	     * Try again with a 16-bit bus.
	     */
	    memctl->memc_br2 |= BR_PS_16;
	    bd->bi_sramsize = ram_size((ulong*)CONFIG_SYS_SRAM_BASE, CONFIG_SYS_SRAM_SIZE);
	}

	if (bd->bi_sramsize == 0) {
	    memctl->memc_br2 = 0;	/* disable select since nothing there */
	}
	else {
	    /* adjust or2 for actual size of SRAM */
	    memctl->memc_or2 |= ORMASK(bd->bi_sramsize);
	    bd->bi_sramstart = CONFIG_SYS_SRAM_BASE;
	    printf("SRAM:  %lu KB\n", bd->bi_sramsize >> 10);
	}


	/* set standard MPC8xx clock so kernel will see the time
	 * even if it doesn't have a DS1306 clock driver.
	 * This helps with experimenting with standard kernels.
	 */
	{
	    ulong tim;
	    struct rtc_time tmp;

	    rtc_get(&tmp);	/* get time from DS1306 RTC */

	    /* convert to seconds since 1970 */
	    tim = mktime(tmp.tm_year, tmp.tm_mon, tmp.tm_mday,
			 tmp.tm_hour, tmp.tm_min, tmp.tm_sec);

	    immap->im_sitk.sitk_rtck = KAPWR_KEY;
	    immap->im_sit.sit_rtc = tim;
	}

	/* set up ethernet address for SCC ethernet. If eth1addr
	 * is present it gets a unique address, otherwise it
	 * shares the FEC address.
	 */
	if (!eth_getenv_enetaddr("eth1addr", enetaddr)) {
		eth_getenv_enetaddr("ethaddr", enetaddr);
		eth_setenv_enetaddr("eth1addr", enetaddr);
	}

	return (0);
}

#if defined(CONFIG_CMD_NAND)
void nand_init(void)
{
	unsigned long totlen = nand_probe(CONFIG_SYS_DFLASH_BASE);

	printf ("%4lu MB\n", totlen >> 20);
}
#endif

/* ------------------------------------------------------------------------- */

/*
 * Check memory range for valid RAM. A simple memory test determines
 * the actually available RAM size between addresses `base' and
 * `base + maxsize'.
 *
 * The memory size MUST be a power of 2 for this to work.
 *
 * The only memory modified is 8 bytes at offset 0. This is important
 * since for the SRAM this location is reserved for autosizing, so if
 * it is modified and the board is reset before ram_size() completes
 * no damage is  done. Normally even the memory at 0 is preserved. The
 * higher SRAM addresses may contain battery backed RAM disk data which
 * must never be corrupted.
 */

static long ram_size(ulong *base, long maxsize)
{
    volatile long	*test_addr;
    volatile ulong	*base_addr = base;
    ulong		ofs;		/* byte offset from base_addr */
    ulong		save;		/* to make test non-destructive */
    ulong		save2;		/* to make test non-destructive */
    long		ramsize = -1;	/* size not determined yet */

    save = *base_addr;		/* save value at 0 so can restore */
    save2 = *(base_addr+1);	/* save value at 4 so can restore */

    /* is any SRAM present? */
    *base_addr = 0x5555aaaa;

    /* It is important to drive the data bus with different data so
     * it doesn't remember the value and look like RAM that isn't there.
     */
    *(base_addr + 1) = 0xaaaa5555;	/* use write to modify data bus */

    if (*base_addr != 0x5555aaaa)
	ramsize = 0;		/* no RAM present, or defective */
    else {
	*base_addr = 0xaaaa5555;
	*(base_addr + 1) = 0x5555aaaa;	/* use write to modify data bus */
	if (*base_addr != 0xaaaa5555)
	    ramsize = 0;	/* no RAM present, or defective */
    }

    /* now size it if any is present */
    for (ofs = 4; ofs < maxsize && ramsize < 0; ofs <<= 1) {
	test_addr = (long*)((long)base_addr + ofs);	/* location to test */

	*base_addr = ~*test_addr;
	if (*base_addr == *test_addr)
	    ramsize = ofs;	/* wrapped back to 0, so this is the size */
    }

    *base_addr = save;		/* restore value at 0 */
    *(base_addr+1) = save2;	/* restore value at 4 */
    return (ramsize);
}

/* ------------------------------------------------------------------------- */
/* sdram table based on the FADS manual                                      */
/* for chip MB811171622A-100                                                 */

/* this table is for 50MHz operation, it should work at all lower speeds */

const uint sdram_table[] =
{
	/* single read. (offset 0 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x11adfc04, 0xefbbbc00,
	0x1ff77c47,

	/* precharge and Mode Register Set initialization (offset 5).
	 * This is also entered at offset 6 to do Mode Register Set
	 * without the precharge.
	 */
	0x1ff77c34, 0xefeabc34, 0x1fb57c35,

	/* burst read. (offset 8 in upm RAM) */
	0x1f07fc04, 0xeeaefc04, 0x10adfc04, 0xf0affc00,
	0xf0affc00, 0xf1affc00, 0xefbbbc00, 0x1ff77c47,
	_not_used_, _not_used_, _not_used_, _not_used_,
	_not_used_, _not_used_, _not_used_, _not_used_,

	/* single write. (offset 18 in upm RAM) */
	/* FADS had 0x1f27fc04, ...
	 * but most other boards have 0x1f07fc04, which
	 * sets GPL0 from A11MPC to 0 1/4 clock earlier,
	 * like the single read.
	 * This seems better so I am going with the change.
	 */
	0x1f07fc04, 0xeeaebc00, 0x01b93c04, 0x1ff77c47,
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
	0x7ffffc07, _not_used_, _not_used_, _not_used_ };

/* ------------------------------------------------------------------------- */

#define	SDRAM_MAX_SIZE		0x10000000	/* max 256 MB SDRAM */

/* precharge and set Mode Register */
#define SDRAM_MCR_PRE    (MCR_OP_RUN | MCR_UPM_A |	/* select UPM     */ \
			  MCR_MB_CS3 |			/* chip select    */ \
			  MCR_MLCF(1) | MCR_MAD(5))	/* 1 time at 0x05 */

/* set Mode Register, no precharge */
#define SDRAM_MCR_MRS    (MCR_OP_RUN | MCR_UPM_A |	/* select UPM     */ \
			  MCR_MB_CS3 |			/* chip select    */ \
			  MCR_MLCF(1) | MCR_MAD(6))	/* 1 time at 0x06 */

/* runs refresh loop twice so get 8 refresh cycles */
#define SDRAM_MCR_REFR   (MCR_OP_RUN | MCR_UPM_A |	/* select UPM     */ \
			  MCR_MB_CS3 |			/* chip select    */ \
			  MCR_MLCF(2) | MCR_MAD(0x30))	/* twice at 0x30  */

/* MAMR values work in either mamr or mbmr */
#define SDRAM_MAMR_BASE  /* refresh at 50MHz */				  \
			 ((195 << MAMR_PTA_SHIFT) | MAMR_PTAE		  \
			 | MAMR_DSA_1_CYCL	/* 1 cycle disable */	  \
			 | MAMR_RLFA_1X		/* Read loop 1 time */	  \
			 | MAMR_WLFA_1X		/* Write loop 1 time */	  \
			 | MAMR_TLFA_4X)	/* Timer loop 4 times */
/* 8 column SDRAM */
#define SDRAM_MAMR_8COL	(SDRAM_MAMR_BASE				  \
			 | MAMR_AMA_TYPE_0	/* Address MUX 0 */	  \
			 | MAMR_G0CLA_A11)	/* GPL0 A11[MPC] */

/* 9 column SDRAM */
#define SDRAM_MAMR_9COL	(SDRAM_MAMR_BASE				  \
			 | MAMR_AMA_TYPE_1	/* Address MUX 1 */	  \
			 | MAMR_G0CLA_A10)	/* GPL0 A10[MPC] */

/* base address 0, 32-bit port, SDRAM UPM, valid */
#define SDRAM_BR_VALUE   (BR_PS_32 | BR_MS_UPMA | BR_V)

/*  up to 256MB, SAM, G5LS - will be adjusted for actual size */
#define SDRAM_OR_PRELIM  (ORMASK(SDRAM_MAX_SIZE) | OR_CSNT_SAM | OR_G5LS)

/* This is the Mode Select Register value for the SDRAM.
 * Burst length: 4
 * Burst Type: sequential
 * CAS Latency: 2
 * Write Burst Length: burst
 */
#define SDRAM_MODE   0x22	/* CAS latency 2, burst length 4 */

/* ------------------------------------------------------------------------- */

phys_size_t initdram(int board_type)
{
	volatile immap_t     *immap = (immap_t *)CONFIG_SYS_IMMR;
	volatile memctl8xx_t *memctl = &immap->im_memctl;
	uint size_sdram = 0;
	uint size_sdram9 = 0;
	uint base = 0;		/* SDRAM must start at 0 */
	int i;

	upmconfig(UPMA, (uint *)sdram_table, sizeof(sdram_table)/sizeof(uint));

	/* Configure the refresh (mostly).  This needs to be
	 * based upon processor clock speed and optimized to provide
	 * the highest level of performance.
	 *
	 * Preliminary prescaler for refresh.
	 * This value is selected for four cycles in 31.2 us,
	 * which gives 8192 cycles in 64 milliseconds.
	 * This may be too fast, but works for any memory.
	 * It is adjusted to 4096 cycles in 64 milliseconds if
	 * possible once we know what memory we have.
	 *
	 * We have to be careful changing UPM registers after we
	 * ask it to run these commands.
	 *
	 * PTA - periodic timer period for our design is
	 *       50 MHz x 31.2us
	 *       ---------------  = 195
	 *       1 x 8 x 1
	 *
	 *    50MHz clock
	 *    31.2us refresh interval
	 *    SCCR[DFBRG] 0
	 *    PTP divide by 8
	 *    1 chip select
	 */
	memctl->memc_mptpr = MPTPR_PTP_DIV8;	/* 0x0800 */
	memctl->memc_mamr = SDRAM_MAMR_8COL & (~MAMR_PTAE); /* no refresh yet */

	/* The SDRAM Mode Register value is shifted left 2 bits since
	 * A30 and A31 don't connect to the SDRAM for 32-bit wide memory.
	 */
	memctl->memc_mar = SDRAM_MODE << 2;	/* MRS code */
	udelay(200);		/* SDRAM needs 200uS before set it up */

	/* Now run the precharge/nop/mrs commands. */
	memctl->memc_mcr = SDRAM_MCR_PRE;
	udelay(2);

	/* Run 8 refresh cycles (2 sets of 4) */
	memctl->memc_mcr = SDRAM_MCR_REFR;	/* run refresh twice */
	udelay(2);

	/* some brands want Mode Register set after the refresh
	 * cycles. This shouldn't hurt anything for the brands
	 * that were happy with the first time we set it.
	 */
	memctl->memc_mcr = SDRAM_MCR_MRS;
	udelay(2);

	memctl->memc_mamr = SDRAM_MAMR_8COL;	/* enable refresh */
	memctl->memc_or3 = SDRAM_OR_PRELIM;
	memctl->memc_br3 = SDRAM_BR_VALUE + base;

	/* Some brands need at least 10 DRAM accesses to stabilize.
	 * It wont hurt the brands that don't.
	 */
	for (i=0; i<10; ++i) {
		volatile ulong *addr = (volatile ulong *)base;
		ulong val;

		val = *(addr + i);
		*(addr + i) = val;
	}

	/* Check SDRAM memory Size in 8 column mode.
	 * For a 9 column memory we will get half the actual size.
	 */
	size_sdram = ram_size((ulong *)0, SDRAM_MAX_SIZE);

	/* Check SDRAM memory Size in 9 column mode.
	 * For an 8 column memory we will see at most 4 megabytes.
	 */
	memctl->memc_mamr = SDRAM_MAMR_9COL;
	size_sdram9 = ram_size((ulong *)0, SDRAM_MAX_SIZE);

	if (size_sdram < size_sdram9)	/* leave configuration at 9 columns */
		size_sdram = size_sdram9;
	else				/* go back to 8 columns */
		memctl->memc_mamr = SDRAM_MAMR_8COL;

	/* adjust or3 for actual size of SDRAM
	 */
	memctl->memc_or3 |= ORMASK(size_sdram);

	/* Adjust refresh rate depending on SDRAM type.
	 * For types > 128 MBit (32 Mbyte for 2 x16 devices) leave
	 * it at the current (fast) rate.
	 * For 16, 64 and 128 MBit half the rate will do.
	 */
	if (size_sdram <= 32 * 1024 * 1024)
		memctl->memc_mptpr = MPTPR_PTP_DIV16;	/* 0x0400 */

	return (size_sdram);
}

/*
 * arch/powerpc/cpu/ppc4xx/4xx_ibm_ddr2_autocalib.c
 * This SPD SDRAM detection code supports AMCC PPC44x cpu's with a
 * DDR2 controller (non Denali Core). Those currently are:
 *
 * 405:		405EX
 * 440/460:	440SP/440SPe/460EX/460GT/460SX
 *
 * (C) Copyright 2008 Applied Micro Circuits Corporation
 * Adam Graham  <agraham@amcc.com>
 *
 * (C) Copyright 2007-2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * COPYRIGHT   AMCC   CORPORATION 2004
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
 *
 */

/* define DEBUG for debugging output (obviously ;-)) */
#undef DEBUG

#include <common.h>
#include <asm/ppc4xx.h>
#include <asm/io.h>
#include <asm/processor.h>

#include "ecc.h"

/*
 * Only compile the DDR auto-calibration code for NOR boot and
 * not for NAND boot (NAND SPL and NAND U-Boot - NUB)
 */
#if !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL)

#define MAXBXCF			4
#define SDRAM_RXBAS_SHIFT_1M	20

#if defined(CONFIG_SYS_DECREMENT_PATTERNS)
#define NUMMEMTESTS		24
#else
#define NUMMEMTESTS		8
#endif /* CONFIG_SYS_DECREMENT_PATTERNS */
#define NUMLOOPS		1	/* configure as you deem approporiate */
#define NUMMEMWORDS		16

#define SDRAM_RDCC_RDSS_VAL(n)	SDRAM_RDCC_RDSS_DECODE(ddr_rdss_opt(n))

/* Private Structure Definitions */

struct autocal_regs {
	u32 rffd;
	u32 rqfd;
};

struct ddrautocal {
	u32 rffd;
	u32 rffd_min;
	u32 rffd_max;
	u32 rffd_size;
	u32 rqfd;
	u32 rqfd_size;
	u32 rdcc;
	u32 flags;
};

struct sdram_timing_clks {
	u32 wrdtr;
	u32 clktr;
	u32 rdcc;
	u32 flags;
};

struct autocal_clks {
	struct sdram_timing_clks clocks;
	struct ddrautocal	 autocal;
};

/*--------------------------------------------------------------------------+
 * Prototypes
 *--------------------------------------------------------------------------*/
#if defined(CONFIG_PPC4xx_DDR_METHOD_A)
static u32 DQS_calibration_methodA(struct ddrautocal *);
static u32 program_DQS_calibration_methodA(struct ddrautocal *);
#else
static u32 DQS_calibration_methodB(struct ddrautocal *);
static u32 program_DQS_calibration_methodB(struct ddrautocal *);
#endif
static int short_mem_test(u32 *);

/*
 * To provide an interface for board specific config values in this common
 * DDR setup code, we implement he "weak" default functions here. They return
 * the default value back to the caller.
 *
 * Please see include/configs/yucca.h for an example fora board specific
 * implementation.
 */

#if !defined(CONFIG_SPD_EEPROM)
u32 __ddr_wrdtr(u32 default_val)
{
	return default_val;
}
u32 ddr_wrdtr(u32) __attribute__((weak, alias("__ddr_wrdtr")));

u32 __ddr_clktr(u32 default_val)
{
	return default_val;
}
u32 ddr_clktr(u32) __attribute__((weak, alias("__ddr_clktr")));

/*
 * Board-specific Platform code can reimplement spd_ddr_init_hang () if needed
 */
void __spd_ddr_init_hang(void)
{
	hang();
}
void
spd_ddr_init_hang(void) __attribute__((weak, alias("__spd_ddr_init_hang")));
#endif /* defined(CONFIG_SPD_EEPROM) */

struct sdram_timing *__ddr_scan_option(struct sdram_timing *default_val)
{
	return default_val;
}
struct sdram_timing *ddr_scan_option(struct sdram_timing *)
	__attribute__((weak, alias("__ddr_scan_option")));

u32 __ddr_rdss_opt(u32 default_val)
{
	return default_val;
}
u32 ddr_rdss_opt(ulong) __attribute__((weak, alias("__ddr_rdss_opt")));


static u32 *get_membase(int bxcr_num)
{
	u32 *membase;

#if defined(SDRAM_R0BAS)
	/* BAS from Memory Queue rank reg. */
	membase =
	    (u32 *)(SDRAM_RXBAS_SDBA_DECODE(mfdcr_any(SDRAM_R0BAS+bxcr_num)));
#else
	{
		ulong bxcf;

		/* BAS from SDRAM_MBxCF mem rank reg. */
		mfsdram(SDRAM_MB0CF + (bxcr_num<<2), bxcf);
		membase = (u32 *)((bxcf & 0xfff80000) << 3);
	}
#endif

	return membase;
}

static inline void ecc_clear_status_reg(void)
{
	mtsdram(SDRAM_ECCES, 0xffffffff);
#if defined(SDRAM_R0BAS)
	mtdcr(SDRAM_ERRSTATLL, 0xffffffff);
#endif
}

/*
 * Reset and relock memory DLL after SDRAM_CLKTR change
 */
static inline void relock_memory_DLL(void)
{
	u32 reg;

	mtsdram(SDRAM_MCOPT2, SDRAM_MCOPT2_IPTR_EXECUTE);

	do {
		mfsdram(SDRAM_MCSTAT, reg);
	} while (!(reg & SDRAM_MCSTAT_MIC_COMP));

	mfsdram(SDRAM_MCOPT2, reg);
	mtsdram(SDRAM_MCOPT2, reg | SDRAM_MCOPT2_DCEN_ENABLE);
}

static int ecc_check_status_reg(void)
{
	u32 ecc_status;

	/*
	 * Compare suceeded, now check
	 * if got ecc error. If got an
	 * ecc error, then don't count
	 * this as a passing value
	 */
	mfsdram(SDRAM_ECCES, ecc_status);
	if (ecc_status != 0x00000000) {
		/* clear on error */
		ecc_clear_status_reg();
		/* ecc check failure */
		return 0;
	}
	ecc_clear_status_reg();
	sync();

	return 1;
}

/* return 1 if passes, 0 if fail */
static int short_mem_test(u32 *base_address)
{
	int i, j, l;
	u32 ecc_mode = 0;

	ulong test[NUMMEMTESTS][NUMMEMWORDS] = {
	/* 0 */	{0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF},
	/* 1 */	{0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000},
	/* 2 */	{0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		 0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		 0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		 0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555},
	/* 3 */	{0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		 0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		 0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		 0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA},
	/* 4 */	{0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		 0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		 0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		 0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A},
	/* 5 */	{0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		 0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		 0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		 0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5},
	/* 6 */	{0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		 0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		 0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		 0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA},
	/* 7 */	{0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		 0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		 0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		 0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55},

#if defined(CONFIG_SYS_DECREMENT_PATTERNS)
	/* 8 */	{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
		 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},
	/* 9 */	{0xfffefffe, 0xfffefffe, 0xfffefffe, 0xfffefffe,
		 0xfffefffe, 0xfffefffe, 0xfffefffe, 0xfffefffe,
		 0xfffefffe, 0xfffefffe, 0xfffefffe, 0xfffefffe,
		 0xfffefffe, 0xfffefffe, 0xfffefffe, 0xfffefffe},
	/* 10 */{0xfffdfffd, 0xfffdfffd, 0xfffdffff, 0xfffdfffd,
		 0xfffdfffd, 0xfffdfffd, 0xfffdffff, 0xfffdfffd,
		 0xfffdfffd, 0xfffdfffd, 0xfffdffff, 0xfffdfffd,
		 0xfffdfffd, 0xfffdfffd, 0xfffdffff, 0xfffdfffd},
	/* 11 */{0xfffcfffc, 0xfffcfffc, 0xfffcfffc, 0xfffcfffc,
		 0xfffcfffc, 0xfffcfffc, 0xfffcfffc, 0xfffcfffc,
		 0xfffcfffc, 0xfffcfffc, 0xfffcfffc, 0xfffcfffc,
		 0xfffcfffc, 0xfffcfffc, 0xfffcfffc, 0xfffcfffc},
	/* 12 */{0xfffbfffb, 0xfffffffb, 0xfffffffb, 0xfffffffb,
		 0xfffbfffb, 0xfffffffb, 0xfffffffb, 0xfffffffb,
		 0xfffbfffb, 0xfffffffb, 0xfffffffb, 0xfffffffb,
		 0xfffbfffb, 0xfffffffb, 0xfffffffb, 0xfffffffb},
	/* 13 */{0xfffafffa, 0xfffafffa, 0xfffffffa, 0xfffafffa,
		 0xfffafffa, 0xfffafffa, 0xfffafffa, 0xfffafffa,
		 0xfffafffa, 0xfffafffa, 0xfffafffa, 0xfffafffa,
		 0xfffafffa, 0xfffafffa, 0xfffafffa, 0xfffafffa},
	/* 14 */{0xfff9fff9, 0xfff9fff9, 0xfff9fff9, 0xfff9fff9,
		 0xfff9fff9, 0xfff9fff9, 0xfff9fff9, 0xfff9fff9,
		 0xfff9fff9, 0xfff9fff9, 0xfff9fff9, 0xfff9fff9,
		 0xfff9fff9, 0xfff9fff9, 0xfff9fff9, 0xfff9fff9},
	/* 15 */{0xfff8fff8, 0xfff8fff8, 0xfff8fff8, 0xfff8fff8,
		 0xfff8fff8, 0xfff8fff8, 0xfff8fff8, 0xfff8fff8,
		 0xfff8fff8, 0xfff8fff8, 0xfff8fff8, 0xfff8fff8,
		 0xfff8fff8, 0xfff8fff8, 0xfff8fff8, 0xfff8fff8},
	/* 16 */{0xfff7fff7, 0xfff7ffff, 0xfff7fff7, 0xfff7fff7,
		 0xfff7fff7, 0xfff7ffff, 0xfff7fff7, 0xfff7fff7,
		 0xfff7fff7, 0xfff7ffff, 0xfff7fff7, 0xfff7fff7,
		 0xfff7ffff, 0xfff7ffff, 0xfff7fff7, 0xfff7fff7},
	/* 17 */{0xfff6fff5, 0xfff6ffff, 0xfff6fff6, 0xfff6fff7,
		 0xfff6fff5, 0xfff6ffff, 0xfff6fff6, 0xfff6fff7,
		 0xfff6fff5, 0xfff6ffff, 0xfff6fff6, 0xfff6fff7,
		 0xfff6fff5, 0xfff6ffff, 0xfff6fff6, 0xfff6fff7},
	/* 18 */{0xfff5fff4, 0xfff5ffff, 0xfff5fff5, 0xfff5fff5,
		 0xfff5fff4, 0xfff5ffff, 0xfff5fff5, 0xfff5fff5,
		 0xfff5fff4, 0xfff5ffff, 0xfff5fff5, 0xfff5fff5,
		 0xfff5fff4, 0xfff5ffff, 0xfff5fff5, 0xfff5fff5},
	/* 19 */{0xfff4fff3, 0xfff4ffff, 0xfff4fff4, 0xfff4fff4,
		 0xfff4fff3, 0xfff4ffff, 0xfff4fff4, 0xfff4fff4,
		 0xfff4fff3, 0xfff4ffff, 0xfff4fff4, 0xfff4fff4,
		 0xfff4fff3, 0xfff4ffff, 0xfff4fff4, 0xfff4fff4},
	/* 20 */{0xfff3fff2, 0xfff3ffff, 0xfff3fff3, 0xfff3fff3,
		 0xfff3fff2, 0xfff3ffff, 0xfff3fff3, 0xfff3fff3,
		 0xfff3fff2, 0xfff3ffff, 0xfff3fff3, 0xfff3fff3,
		 0xfff3fff2, 0xfff3ffff, 0xfff3fff3, 0xfff3fff3},
	/* 21 */{0xfff2ffff, 0xfff2ffff, 0xfff2fff2, 0xfff2fff2,
		 0xfff2ffff, 0xfff2ffff, 0xfff2fff2, 0xfff2fff2,
		 0xfff2ffff, 0xfff2ffff, 0xfff2fff2, 0xfff2fff2,
		 0xfff2ffff, 0xfff2ffff, 0xfff2fff2, 0xfff2fff2},
	/* 22 */{0xfff1ffff, 0xfff1ffff, 0xfff1fff1, 0xfff1fff1,
		 0xfff1ffff, 0xfff1ffff, 0xfff1fff1, 0xfff1fff1,
		 0xfff1ffff, 0xfff1ffff, 0xfff1fff1, 0xfff1fff1,
		 0xfff1ffff, 0xfff1ffff, 0xfff1fff1, 0xfff1fff1},
	/* 23 */{0xfff0fff0, 0xfff0fff0, 0xfff0fff0, 0xfff0fff0,
		 0xfff0fff0, 0xfff0fff0, 0xfff0fff0, 0xfff0fff0,
		 0xfff0fff0, 0xfff0fff0, 0xfff0fff0, 0xfff0fff0,
		 0xfff0fff0, 0xfff0fffe, 0xfff0fff0, 0xfff0fff0},
#endif /* CONFIG_SYS_DECREMENT_PATTERNS */
								 };

	mfsdram(SDRAM_MCOPT1, ecc_mode);
	if ((ecc_mode & SDRAM_MCOPT1_MCHK_CHK_REP) ==
						SDRAM_MCOPT1_MCHK_CHK_REP) {
		ecc_clear_status_reg();
		sync();
		ecc_mode = 1;
	} else {
		ecc_mode = 0;
	}

	/*
	 * Run the short memory test.
	 */
	for (i = 0; i < NUMMEMTESTS; i++) {
		for (j = 0; j < NUMMEMWORDS; j++) {
			base_address[j] = test[i][j];
			ppcDcbf((ulong)&(base_address[j]));
		}
		sync();
		iobarrier_rw();
		for (l = 0; l < NUMLOOPS; l++) {
			for (j = 0; j < NUMMEMWORDS; j++) {
				if (base_address[j] != test[i][j]) {
					ppcDcbf((u32)&(base_address[j]));
					return 0;
				} else {
					if (ecc_mode) {
						if (!ecc_check_status_reg())
							return 0;
					}
				}
				ppcDcbf((u32)&(base_address[j]));
			} /* for (j = 0; j < NUMMEMWORDS; j++) */
			sync();
			iobarrier_rw();
		} /* for (l=0; l<NUMLOOPS; l++) */
	}

	return 1;
}

#if defined(CONFIG_PPC4xx_DDR_METHOD_A)
/*-----------------------------------------------------------------------------+
| program_DQS_calibration_methodA.
+-----------------------------------------------------------------------------*/
static u32 program_DQS_calibration_methodA(struct ddrautocal *ddrcal)
{
	u32 pass_result = 0;

#ifdef DEBUG
	ulong temp;

	mfsdram(SDRAM_RDCC, temp);
	debug("<%s>SDRAM_RDCC=0x%08x\n", __func__, temp);
#endif

	pass_result = DQS_calibration_methodA(ddrcal);

	return pass_result;
}

/*
 * DQS_calibration_methodA()
 *
 * Autocalibration Method A
 *
 *  ARRAY [Entire DQS Range] DQS_Valid_Window ;    initialized to all zeros
 *  ARRAY [Entire FDBK Range] FDBK_Valid_Window;   initialized to all zeros
 *  MEMWRITE(addr, expected_data);
 *  for (i = 0; i < Entire DQS Range; i++) {       RQDC.RQFD
 *      for (j = 0; j < Entire FDBK Range; j++) {  RFDC.RFFD
 *         MEMREAD(addr, actual_data);
 *         if (actual_data == expected_data) {
 *             DQS_Valid_Window[i] = 1;            RQDC.RQFD
 *             FDBK_Valid_Window[i][j] = 1;        RFDC.RFFD
 *         }
 *      }
 *  }
 */
static u32 DQS_calibration_methodA(struct ddrautocal *cal)
{
	ulong rfdc_reg;
	ulong rffd;

	ulong rqdc_reg;
	ulong rqfd;

	u32 *membase;
	ulong bxcf;
	int rqfd_average;
	int bxcr_num;
	int rffd_average;
	int pass;
	u32 passed = 0;

	int in_window;
	struct autocal_regs curr_win_min;
	struct autocal_regs curr_win_max;
	struct autocal_regs best_win_min;
	struct autocal_regs best_win_max;
	struct autocal_regs loop_win_min;
	struct autocal_regs loop_win_max;

#ifdef DEBUG
	ulong temp;
#endif
	ulong rdcc;

	char slash[] = "\\|/-\\|/-";
	int loopi = 0;

	/* start */
	in_window = 0;

	memset(&curr_win_min, 0, sizeof(curr_win_min));
	memset(&curr_win_max, 0, sizeof(curr_win_max));
	memset(&best_win_min, 0, sizeof(best_win_min));
	memset(&best_win_max, 0, sizeof(best_win_max));
	memset(&loop_win_min, 0, sizeof(loop_win_min));
	memset(&loop_win_max, 0, sizeof(loop_win_max));

	rdcc = 0;

	/*
	 * Program RDCC register
	 * Read sample cycle auto-update enable
	 */
	mtsdram(SDRAM_RDCC,
		ddr_rdss_opt(SDRAM_RDCC_RDSS_T2) | SDRAM_RDCC_RSAE_ENABLE);

#ifdef DEBUG
	mfsdram(SDRAM_RDCC, temp);
	debug("<%s>SDRAM_RDCC=0x%x\n", __func__, temp);
	mfsdram(SDRAM_RTSR, temp);
	debug("<%s>SDRAM_RTSR=0x%x\n", __func__, temp);
	mfsdram(SDRAM_FCSR, temp);
	debug("<%s>SDRAM_FCSR=0x%x\n", __func__, temp);
#endif

	/*
	 * Program RQDC register
	 * Internal DQS delay mechanism enable
	 */
	mtsdram(SDRAM_RQDC,
		SDRAM_RQDC_RQDE_ENABLE | SDRAM_RQDC_RQFD_ENCODE(0x00));

#ifdef DEBUG
	mfsdram(SDRAM_RQDC, temp);
	debug("<%s>SDRAM_RQDC=0x%x\n", __func__, temp);
#endif

	/*
	 * Program RFDC register
	 * Set Feedback Fractional Oversample
	 * Auto-detect read sample cycle enable
	 */
	mtsdram(SDRAM_RFDC, SDRAM_RFDC_ARSE_ENABLE |
		SDRAM_RFDC_RFOS_ENCODE(0) | SDRAM_RFDC_RFFD_ENCODE(0));

#ifdef DEBUG
	mfsdram(SDRAM_RFDC, temp);
	debug("<%s>SDRAM_RFDC=0x%x\n", __func__, temp);
#endif

	putc(' ');
	for (rqfd = 0; rqfd <= SDRAM_RQDC_RQFD_MAX; rqfd++) {

		mfsdram(SDRAM_RQDC, rqdc_reg);
		rqdc_reg &= ~(SDRAM_RQDC_RQFD_MASK);
		mtsdram(SDRAM_RQDC, rqdc_reg | SDRAM_RQDC_RQFD_ENCODE(rqfd));

		putc('\b');
		putc(slash[loopi++ % 8]);

		curr_win_min.rffd = 0;
		curr_win_max.rffd = 0;
		in_window = 0;

		for (rffd = 0, pass = 0; rffd <= SDRAM_RFDC_RFFD_MAX; rffd++) {
			mfsdram(SDRAM_RFDC, rfdc_reg);
			rfdc_reg &= ~(SDRAM_RFDC_RFFD_MASK);
			mtsdram(SDRAM_RFDC,
				    rfdc_reg | SDRAM_RFDC_RFFD_ENCODE(rffd));

			for (bxcr_num = 0; bxcr_num < MAXBXCF; bxcr_num++) {
				mfsdram(SDRAM_MB0CF + (bxcr_num<<2), bxcf);

				/* Banks enabled */
				if (bxcf & SDRAM_BXCF_M_BE_MASK) {
					/* Bank is enabled */
					membase = get_membase(bxcr_num);
					pass = short_mem_test(membase);
				} /* if bank enabled */
			} /* for bxcr_num */

			/* If this value passed update RFFD windows */
			if (pass && !in_window) { /* at the start of window */
				in_window = 1;
				curr_win_min.rffd = curr_win_max.rffd = rffd;
				curr_win_min.rqfd = curr_win_max.rqfd = rqfd;
				mfsdram(SDRAM_RDCC, rdcc); /*record this value*/
			} else if (!pass && in_window) { /* at end of window */
				in_window = 0;
			} else if (pass && in_window) { /* within the window */
				curr_win_max.rffd = rffd;
				curr_win_max.rqfd = rqfd;
			}
			/* else if (!pass && !in_window)
				skip - no pass, not currently in a window */

			if (in_window) {
				if ((curr_win_max.rffd - curr_win_min.rffd) >
				    (best_win_max.rffd - best_win_min.rffd)) {
					best_win_min.rffd = curr_win_min.rffd;
					best_win_max.rffd = curr_win_max.rffd;

					best_win_min.rqfd = curr_win_min.rqfd;
					best_win_max.rqfd = curr_win_max.rqfd;
					cal->rdcc	  = rdcc;
				}
				passed = 1;
			}
		} /* RFDC.RFFD */

		/*
		 * save-off the best window results of the RFDC.RFFD
		 * for this RQDC.RQFD setting
		 */
		/*
		 * if (just ended RFDC.RFDC loop pass window) >
		 *	(prior RFDC.RFFD loop pass window)
		 */
		if ((best_win_max.rffd - best_win_min.rffd) >
		    (loop_win_max.rffd - loop_win_min.rffd)) {
			loop_win_min.rffd = best_win_min.rffd;
			loop_win_max.rffd = best_win_max.rffd;
			loop_win_min.rqfd = rqfd;
			loop_win_max.rqfd = rqfd;
			debug("RQFD.min 0x%08x, RQFD.max 0x%08x, "
			      "RFFD.min 0x%08x, RFFD.max 0x%08x\n",
					loop_win_min.rqfd, loop_win_max.rqfd,
					loop_win_min.rffd, loop_win_max.rffd);
		}
	} /* RQDC.RQFD */

	putc('\b');

	debug("\n");

	if ((loop_win_min.rffd == 0) && (loop_win_max.rffd == 0) &&
	    (best_win_min.rffd == 0) && (best_win_max.rffd == 0) &&
	    (best_win_min.rqfd == 0) && (best_win_max.rqfd == 0)) {
		passed = 0;
	}

	/*
	 * Need to program RQDC before RFDC.
	 */
	debug("<%s> RQFD Min: 0x%x\n", __func__, loop_win_min.rqfd);
	debug("<%s> RQFD Max: 0x%x\n", __func__, loop_win_max.rqfd);
	rqfd_average = loop_win_max.rqfd;

	if (rqfd_average < 0)
		rqfd_average = 0;

	if (rqfd_average > SDRAM_RQDC_RQFD_MAX)
		rqfd_average = SDRAM_RQDC_RQFD_MAX;

	debug("<%s> RFFD average: 0x%08x\n", __func__, rqfd_average);
	mtsdram(SDRAM_RQDC, (rqdc_reg & ~SDRAM_RQDC_RQFD_MASK) |
				SDRAM_RQDC_RQFD_ENCODE(rqfd_average));

	debug("<%s> RFFD Min: 0x%08x\n", __func__, loop_win_min.rffd);
	debug("<%s> RFFD Max: 0x%08x\n", __func__, loop_win_max.rffd);
	rffd_average = ((loop_win_min.rffd + loop_win_max.rffd) / 2);

	if (rffd_average < 0)
		rffd_average = 0;

	if (rffd_average > SDRAM_RFDC_RFFD_MAX)
		rffd_average = SDRAM_RFDC_RFFD_MAX;

	debug("<%s> RFFD average: 0x%08x\n", __func__, rffd_average);
	mtsdram(SDRAM_RFDC, rfdc_reg | SDRAM_RFDC_RFFD_ENCODE(rffd_average));

	/* if something passed, then return the size of the largest window */
	if (passed != 0) {
		passed		= loop_win_max.rffd - loop_win_min.rffd;
		cal->rqfd	= rqfd_average;
		cal->rffd	= rffd_average;
		cal->rffd_min	= loop_win_min.rffd;
		cal->rffd_max	= loop_win_max.rffd;
	}

	return (u32)passed;
}

#else	/* !defined(CONFIG_PPC4xx_DDR_METHOD_A) */

/*-----------------------------------------------------------------------------+
| program_DQS_calibration_methodB.
+-----------------------------------------------------------------------------*/
static u32 program_DQS_calibration_methodB(struct ddrautocal *ddrcal)
{
	u32 pass_result = 0;

#ifdef DEBUG
	ulong temp;
#endif

	/*
	 * Program RDCC register
	 * Read sample cycle auto-update enable
	 */
	mtsdram(SDRAM_RDCC,
		ddr_rdss_opt(SDRAM_RDCC_RDSS_T2) | SDRAM_RDCC_RSAE_ENABLE);

#ifdef DEBUG
	mfsdram(SDRAM_RDCC, temp);
	debug("<%s>SDRAM_RDCC=0x%08x\n", __func__, temp);
#endif

	/*
	 * Program RQDC register
	 * Internal DQS delay mechanism enable
	 */
	mtsdram(SDRAM_RQDC,
#if defined(CONFIG_DDR_RQDC_START_VAL)
			SDRAM_RQDC_RQDE_ENABLE |
			    SDRAM_RQDC_RQFD_ENCODE(CONFIG_DDR_RQDC_START_VAL));
#else
			SDRAM_RQDC_RQDE_ENABLE | SDRAM_RQDC_RQFD_ENCODE(0x38));
#endif

#ifdef DEBUG
	mfsdram(SDRAM_RQDC, temp);
	debug("<%s>SDRAM_RQDC=0x%08x\n", __func__, temp);
#endif

	/*
	 * Program RFDC register
	 * Set Feedback Fractional Oversample
	 * Auto-detect read sample cycle enable
	 */
	mtsdram(SDRAM_RFDC,	SDRAM_RFDC_ARSE_ENABLE |
				SDRAM_RFDC_RFOS_ENCODE(0) |
				SDRAM_RFDC_RFFD_ENCODE(0));

#ifdef DEBUG
	mfsdram(SDRAM_RFDC, temp);
	debug("<%s>SDRAM_RFDC=0x%08x\n", __func__, temp);
#endif

	pass_result = DQS_calibration_methodB(ddrcal);

	return pass_result;
}

/*
 * DQS_calibration_methodB()
 *
 * Autocalibration Method B
 *
 * ARRAY [Entire DQS Range] DQS_Valid_Window ;       initialized to all zeros
 * ARRAY [Entire Feedback Range] FDBK_Valid_Window;  initialized to all zeros
 * MEMWRITE(addr, expected_data);
 * Initialialize the DQS delay to 80 degrees (MCIF0_RRQDC[RQFD]=0x38).
 *
 *  for (j = 0; j < Entire Feedback Range; j++) {
 *      MEMREAD(addr, actual_data);
 *       if (actual_data == expected_data) {
 *           FDBK_Valid_Window[j] = 1;
 *       }
 * }
 *
 * Set MCIF0_RFDC[RFFD] to the middle of the FDBK_Valid_Window.
 *
 * for (i = 0; i < Entire DQS Range; i++) {
 *     MEMREAD(addr, actual_data);
 *     if (actual_data == expected_data) {
 *         DQS_Valid_Window[i] = 1;
 *      }
 * }
 *
 * Set MCIF0_RRQDC[RQFD] to the middle of the DQS_Valid_Window.
 */
/*-----------------------------------------------------------------------------+
| DQS_calibration_methodB.
+-----------------------------------------------------------------------------*/
static u32 DQS_calibration_methodB(struct ddrautocal *cal)
{
	ulong rfdc_reg;
#ifndef CONFIG_DDR_RFDC_FIXED
	ulong rffd;
#endif

	ulong rqdc_reg;
	ulong rqfd;

	ulong rdcc;

	u32 *membase;
	ulong bxcf;
	int rqfd_average;
	int bxcr_num;
	int rffd_average;
	int pass;
	uint passed = 0;

	int in_window;
	u32 curr_win_min, curr_win_max;
	u32 best_win_min, best_win_max;
	u32 size = 0;

	/*------------------------------------------------------------------
	 | Test to determine the best read clock delay tuning bits.
	 |
	 | Before the DDR controller can be used, the read clock delay needs to
	 | be set.  This is SDRAM_RQDC[RQFD] and SDRAM_RFDC[RFFD].
	 | This value cannot be hardcoded into the program because it changes
	 | depending on the board's setup and environment.
	 | To do this, all delay values are tested to see if they
	 | work or not.  By doing this, you get groups of fails with groups of
	 | passing values.  The idea is to find the start and end of a passing
	 | window and take the center of it to use as the read clock delay.
	 |
	 | A failure has to be seen first so that when we hit a pass, we know
	 | that it is truely the start of the window.  If we get passing values
	 | to start off with, we don't know if we are at the start of the window
	 |
	 | The code assumes that a failure will always be found.
	 | If a failure is not found, there is no easy way to get the middle
	 | of the passing window.  I guess we can pretty much pick any value
	 | but some values will be better than others.  Since the lowest speed
	 | we can clock the DDR interface at is 200 MHz (2x 100 MHz PLB speed),
	 | from experimentation it is safe to say you will always have a failure
	 +-----------------------------------------------------------------*/

	debug("\n\n");

#if defined(CONFIG_DDR_RFDC_FIXED)
	mtsdram(SDRAM_RFDC, CONFIG_DDR_RFDC_FIXED);
	size = 512;
	rffd_average = CONFIG_DDR_RFDC_FIXED & SDRAM_RFDC_RFFD_MASK;
	mfsdram(SDRAM_RDCC, rdcc);	/* record this value */
	cal->rdcc = rdcc;
#else /* CONFIG_DDR_RFDC_FIXED */
	in_window = 0;
	rdcc = 0;

	curr_win_min = curr_win_max = 0;
	best_win_min = best_win_max = 0;
	for (rffd = 0; rffd <= SDRAM_RFDC_RFFD_MAX; rffd++) {
		mfsdram(SDRAM_RFDC, rfdc_reg);
		rfdc_reg &= ~(SDRAM_RFDC_RFFD_MASK);
		mtsdram(SDRAM_RFDC, rfdc_reg | SDRAM_RFDC_RFFD_ENCODE(rffd));

		pass = 1;
		for (bxcr_num = 0; bxcr_num < MAXBXCF; bxcr_num++) {
			mfsdram(SDRAM_MB0CF + (bxcr_num<<2), bxcf);

			/* Banks enabled */
			if (bxcf & SDRAM_BXCF_M_BE_MASK) {
				/* Bank is enabled */
				membase = get_membase(bxcr_num);
				pass &= short_mem_test(membase);
			} /* if bank enabled */
		} /* for bxcf_num */

		/* If this value passed */
		if (pass && !in_window) {	/* start of passing window */
			in_window = 1;
			curr_win_min = curr_win_max = rffd;
			mfsdram(SDRAM_RDCC, rdcc);	/* record this value */
		} else if (!pass && in_window) {	/* end passing window */
			in_window = 0;
		} else if (pass && in_window) {	/* within the passing window */
			curr_win_max = rffd;
		}

		if (in_window) {
			if ((curr_win_max - curr_win_min) >
			    (best_win_max - best_win_min)) {
				best_win_min = curr_win_min;
				best_win_max = curr_win_max;
				cal->rdcc    = rdcc;
			}
			passed = 1;
		}
	} /* for rffd */

	if ((best_win_min == 0) && (best_win_max == 0))
		passed = 0;
	else
		size = best_win_max - best_win_min;

	debug("RFFD Min: 0x%x\n", best_win_min);
	debug("RFFD Max: 0x%x\n", best_win_max);
	rffd_average = ((best_win_min + best_win_max) / 2);

	cal->rffd_min = best_win_min;
	cal->rffd_max = best_win_max;

	if (rffd_average < 0)
		rffd_average = 0;

	if (rffd_average > SDRAM_RFDC_RFFD_MAX)
		rffd_average = SDRAM_RFDC_RFFD_MAX;

	mtsdram(SDRAM_RFDC, rfdc_reg | SDRAM_RFDC_RFFD_ENCODE(rffd_average));
#endif /* CONFIG_DDR_RFDC_FIXED */

	in_window = 0;

	curr_win_min = curr_win_max = 0;
	best_win_min = best_win_max = 0;
	for (rqfd = 0; rqfd <= SDRAM_RQDC_RQFD_MAX; rqfd++) {
		mfsdram(SDRAM_RQDC, rqdc_reg);
		rqdc_reg &= ~(SDRAM_RQDC_RQFD_MASK);
		mtsdram(SDRAM_RQDC, rqdc_reg | SDRAM_RQDC_RQFD_ENCODE(rqfd));

		pass = 1;
		for (bxcr_num = 0; bxcr_num < MAXBXCF; bxcr_num++) {

			mfsdram(SDRAM_MB0CF + (bxcr_num<<2), bxcf);

			/* Banks enabled */
			if (bxcf & SDRAM_BXCF_M_BE_MASK) {
				/* Bank is enabled */
				membase = get_membase(bxcr_num);
				pass &= short_mem_test(membase);
			} /* if bank enabled */
		} /* for bxcf_num */

		/* If this value passed */
		if (pass && !in_window) {
			in_window = 1;
			curr_win_min = curr_win_max = rqfd;
		} else if (!pass && in_window) {
			in_window = 0;
		} else if (pass && in_window) {
			curr_win_max = rqfd;
		}

		if (in_window) {
			if ((curr_win_max - curr_win_min) >
			    (best_win_max - best_win_min)) {
				best_win_min = curr_win_min;
				best_win_max = curr_win_max;
			}
			passed = 1;
		}
	} /* for rqfd */

	if ((best_win_min == 0) && (best_win_max == 0))
		passed = 0;

	debug("RQFD Min: 0x%x\n", best_win_min);
	debug("RQFD Max: 0x%x\n", best_win_max);
	rqfd_average = ((best_win_min + best_win_max) / 2);

	if (rqfd_average < 0)
		rqfd_average = 0;

	if (rqfd_average > SDRAM_RQDC_RQFD_MAX)
		rqfd_average = SDRAM_RQDC_RQFD_MAX;

	mtsdram(SDRAM_RQDC, (rqdc_reg & ~SDRAM_RQDC_RQFD_MASK) |
					SDRAM_RQDC_RQFD_ENCODE(rqfd_average));

	mfsdram(SDRAM_RQDC, rqdc_reg);
	mfsdram(SDRAM_RFDC, rfdc_reg);

	/*
	 * Need to program RQDC before RFDC. The value is read above.
	 * That is the reason why auto cal not work.
	 * See, comments below.
	 */
	mtsdram(SDRAM_RQDC, rqdc_reg);
	mtsdram(SDRAM_RFDC, rfdc_reg);

	debug("RQDC: 0x%08lX\n", rqdc_reg);
	debug("RFDC: 0x%08lX\n", rfdc_reg);

	/* if something passed, then return the size of the largest window */
	if (passed != 0) {
		passed		= size;
		cal->rqfd	= rqfd_average;
		cal->rffd	= rffd_average;
	}

	return (uint)passed;
}
#endif /* defined(CONFIG_PPC4xx_DDR_METHOD_A) */

/*
 * Default table for DDR auto-calibration of all
 * possible WRDTR and CLKTR values.
 * Table format is:
 *	 {SDRAM_WRDTR.[WDTR], SDRAM_CLKTR.[CKTR]}
 *
 * Table is terminated with {-1, -1} value pair.
 *
 * Board vendors can specify their own board specific subset of
 * known working {SDRAM_WRDTR.[WDTR], SDRAM_CLKTR.[CKTR]} value
 * pairs via a board defined ddr_scan_option() function.
 */
static struct sdram_timing full_scan_options[] = {
	{0, 0}, {0, 1}, {0, 2}, {0, 3},
	{1, 0}, {1, 1}, {1, 2}, {1, 3},
	{2, 0}, {2, 1}, {2, 2}, {2, 3},
	{3, 0}, {3, 1}, {3, 2}, {3, 3},
	{4, 0}, {4, 1}, {4, 2}, {4, 3},
	{5, 0}, {5, 1}, {5, 2}, {5, 3},
	{6, 0}, {6, 1}, {6, 2}, {6, 3},
	{-1, -1}
};

/*---------------------------------------------------------------------------+
| DQS_calibration.
+----------------------------------------------------------------------------*/
u32 DQS_autocalibration(void)
{
	u32 wdtr;
	u32 clkp;
	u32 result = 0;
	u32 best_result = 0;
	u32 best_rdcc;
	struct ddrautocal ddrcal;
	struct autocal_clks tcal;
	ulong rfdc_reg;
	ulong rqdc_reg;
	u32 val;
	int verbose_lvl = 0;
	char *str;
	char slash[] = "\\|/-\\|/-";
	int loopi = 0;
	struct sdram_timing *scan_list;

#if defined(DEBUG_PPC4xx_DDR_AUTOCALIBRATION)
	int i;
	char tmp[64];	/* long enough for environment variables */
#endif

	memset(&tcal, 0, sizeof(tcal));

	scan_list = ddr_scan_option(full_scan_options);

	mfsdram(SDRAM_MCOPT1, val);
	if ((val & SDRAM_MCOPT1_MCHK_CHK_REP) == SDRAM_MCOPT1_MCHK_CHK_REP)
		str = "ECC Auto calibration -";
	else
		str = "Auto calibration -";

	puts(str);

#if defined(DEBUG_PPC4xx_DDR_AUTOCALIBRATION)
	i = getenv_f("autocalib", tmp, sizeof(tmp));
	if (i < 0)
		strcpy(tmp, CONFIG_AUTOCALIB);

	if (strcmp(tmp, "final") == 0) {
		/* display the final autocalibration results only */
		verbose_lvl = 1;
	} else if (strcmp(tmp, "loop") == 0) {
		/* display summary autocalibration info per iteration */
		verbose_lvl = 2;
	} else if (strcmp(tmp, "display") == 0) {
		/* display full debug autocalibration window info. */
		verbose_lvl = 3;
	}
#endif /* (DEBUG_PPC4xx_DDR_AUTOCALIBRATION) */

	best_rdcc = (SDRAM_RDCC_RDSS_T4 >> 30);

	while ((scan_list->wrdtr != -1) && (scan_list->clktr != -1)) {
		wdtr = scan_list->wrdtr;
		clkp = scan_list->clktr;

		mfsdram(SDRAM_WRDTR, val);
		val &= ~(SDRAM_WRDTR_LLWP_MASK | SDRAM_WRDTR_WTR_MASK);
		mtsdram(SDRAM_WRDTR, (val |
			ddr_wrdtr(SDRAM_WRDTR_LLWP_1_CYC | (wdtr << 25))));

		mtsdram(SDRAM_CLKTR, clkp << 30);

		relock_memory_DLL();

		putc('\b');
		putc(slash[loopi++ % 8]);

#ifdef DEBUG
		debug("\n");
		debug("*** --------------\n");
		mfsdram(SDRAM_WRDTR, val);
		debug("*** SDRAM_WRDTR set to 0x%08x\n", val);
		mfsdram(SDRAM_CLKTR, val);
		debug("*** SDRAM_CLKTR set to 0x%08x\n", val);
#endif

		debug("\n");
		if (verbose_lvl > 2) {
			printf("*** SDRAM_WRDTR (wdtr) set to %d\n", wdtr);
			printf("*** SDRAM_CLKTR (clkp) set to %d\n", clkp);
		}

		memset(&ddrcal, 0, sizeof(ddrcal));

		/*
		 * DQS calibration.
		 */
		/*
		 * program_DQS_calibration_method[A|B]() returns 0 if no
		 * passing RFDC.[RFFD] window is found or returns the size
		 * of the best passing window; in the case of a found passing
		 * window, the ddrcal will contain the values of the best
		 * window RQDC.[RQFD] and RFDC.[RFFD].
		 */

		/*
		 * Call PPC4xx SDRAM DDR autocalibration methodA or methodB.
		 * Default is methodB.
		 * Defined the autocalibration method in the board specific
		 * header file.
		 * Please see include/configs/kilauea.h for an example for
		 * a board specific implementation.
		 */
#if defined(CONFIG_PPC4xx_DDR_METHOD_A)
		result = program_DQS_calibration_methodA(&ddrcal);
#else
		result = program_DQS_calibration_methodB(&ddrcal);
#endif

		sync();

		/*
		 * Clear potential errors resulting from auto-calibration.
		 * If not done, then we could get an interrupt later on when
		 * exceptions are enabled.
		 */
		set_mcsr(get_mcsr());

		val = ddrcal.rdcc;	/* RDCC from the best passing window */

		udelay(100);

		if (verbose_lvl > 1) {
			char *tstr;
			switch ((val >> 30)) {
			case 0:
				if (result != 0)
					tstr = "T1";
				else
					tstr = "N/A";
				break;
			case 1:
				tstr = "T2";
				break;
			case 2:
				tstr = "T3";
				break;
			case 3:
				tstr = "T4";
				break;
			default:
				tstr = "unknown";
				break;
			}
			printf("** WRDTR(%d) CLKTR(%d), Wind (%d), best (%d), "
			       "max-min(0x%04x)(0x%04x), RDCC: %s\n",
				wdtr, clkp, result, best_result,
				ddrcal.rffd_min, ddrcal.rffd_max, tstr);
		}

		/*
		 * The DQS calibration "result" is either "0"
		 * if no passing window was found, or is the
		 * size of the RFFD passing window.
		 */
		/*
		 * want the lowest Read Sample Cycle Select
		 */
		val = SDRAM_RDCC_RDSS_DECODE(val);
		debug("*** (%d) (%d) current_rdcc, best_rdcc\n",
			val, best_rdcc);

		if ((result != 0) &&
		    (val >= SDRAM_RDCC_RDSS_VAL(SDRAM_RDCC_RDSS_T2))) {
			if (((result == best_result) && (val < best_rdcc)) ||
			    ((result > best_result) && (val <= best_rdcc))) {
				tcal.autocal.flags = 1;
				debug("*** (%d)(%d) result passed window "
					"size: 0x%08x, rqfd = 0x%08x, "
					"rffd = 0x%08x, rdcc = 0x%08x\n",
					wdtr, clkp, result, ddrcal.rqfd,
					ddrcal.rffd, ddrcal.rdcc);

				/*
				 * Save the SDRAM_WRDTR and SDRAM_CLKTR
				 * settings for the largest returned
				 * RFFD passing window size.
				 */
				best_rdcc = val;
				tcal.clocks.wrdtr = wdtr;
				tcal.clocks.clktr = clkp;
				tcal.clocks.rdcc = SDRAM_RDCC_RDSS_ENCODE(val);
				tcal.autocal.rqfd = ddrcal.rqfd;
				tcal.autocal.rffd = ddrcal.rffd;
				best_result = result;

					if (verbose_lvl > 2) {
						printf("** (%d)(%d)  "
						       "best result: 0x%04x\n",
							wdtr, clkp,
							best_result);
						printf("** (%d)(%d)  "
						       "best WRDTR: 0x%04x\n",
							wdtr, clkp,
							tcal.clocks.wrdtr);
						printf("** (%d)(%d)  "
						       "best CLKTR: 0x%04x\n",
							wdtr, clkp,
							tcal.clocks.clktr);
						printf("** (%d)(%d)  "
						       "best RQDC: 0x%04x\n",
							wdtr, clkp,
							tcal.autocal.rqfd);
						printf("** (%d)(%d)  "
						       "best RFDC: 0x%04x\n",
							wdtr, clkp,
							tcal.autocal.rffd);
						printf("** (%d)(%d)  "
						       "best RDCC: 0x%08x\n",
							wdtr, clkp,
							(u32)tcal.clocks.rdcc);
						mfsdram(SDRAM_RTSR, val);
						printf("** (%d)(%d)  best "
						       "loop RTSR: 0x%08x\n",
							wdtr, clkp, val);
						mfsdram(SDRAM_FCSR, val);
						printf("** (%d)(%d)  best "
						       "loop FCSR: 0x%08x\n",
							wdtr, clkp, val);
					}
			}
		} /* if ((result != 0) && (val >= (ddr_rdss_opt()))) */
		scan_list++;
	} /* while ((scan_list->wrdtr != -1) && (scan_list->clktr != -1)) */

	if (tcal.autocal.flags == 1) {
		if (verbose_lvl > 0) {
			printf("*** --------------\n");
			printf("*** best_result window size: %d\n",
							best_result);
			printf("*** best_result WRDTR: 0x%04x\n",
							tcal.clocks.wrdtr);
			printf("*** best_result CLKTR: 0x%04x\n",
							tcal.clocks.clktr);
			printf("*** best_result RQFD: 0x%04x\n",
							tcal.autocal.rqfd);
			printf("*** best_result RFFD: 0x%04x\n",
							tcal.autocal.rffd);
			printf("*** best_result RDCC: 0x%04x\n",
							tcal.clocks.rdcc);
			printf("*** --------------\n");
			printf("\n");
		}

		/*
		 * if got best passing result window, then lock in the
		 * best CLKTR, WRDTR, RQFD, and RFFD values
		 */
		mfsdram(SDRAM_WRDTR, val);
		mtsdram(SDRAM_WRDTR, (val &
		    ~(SDRAM_WRDTR_LLWP_MASK | SDRAM_WRDTR_WTR_MASK)) |
		    ddr_wrdtr(SDRAM_WRDTR_LLWP_1_CYC |
					(tcal.clocks.wrdtr << 25)));

		mtsdram(SDRAM_CLKTR, tcal.clocks.clktr << 30);

		relock_memory_DLL();

		mfsdram(SDRAM_RQDC, rqdc_reg);
		rqdc_reg &= ~(SDRAM_RQDC_RQFD_MASK);
		mtsdram(SDRAM_RQDC, rqdc_reg |
				SDRAM_RQDC_RQFD_ENCODE(tcal.autocal.rqfd));

		mfsdram(SDRAM_RQDC, rqdc_reg);
		debug("*** best_result: read value SDRAM_RQDC 0x%08lx\n",
				rqdc_reg);

#if defined(CONFIG_DDR_RFDC_FIXED)
		mtsdram(SDRAM_RFDC, CONFIG_DDR_RFDC_FIXED);
#else /* CONFIG_DDR_RFDC_FIXED */
		mfsdram(SDRAM_RFDC, rfdc_reg);
		rfdc_reg &= ~(SDRAM_RFDC_RFFD_MASK);
		mtsdram(SDRAM_RFDC, rfdc_reg |
				SDRAM_RFDC_RFFD_ENCODE(tcal.autocal.rffd));
#endif /* CONFIG_DDR_RFDC_FIXED */

		mfsdram(SDRAM_RFDC, rfdc_reg);
		debug("*** best_result: read value SDRAM_RFDC 0x%08lx\n",
				rfdc_reg);
		mfsdram(SDRAM_RDCC, val);
		debug("***  SDRAM_RDCC 0x%08x\n", val);
	} else {
		/*
		 * no valid windows were found
		 */
		printf("DQS memory calibration window can not be determined, "
		       "terminating u-boot.\n");
		ppc4xx_ibm_ddr2_register_dump();
		spd_ddr_init_hang();
	}

	blank_string(strlen(str));

	return 0;
}
#else /* defined(CONFIG_NAND_U_BOOT) || defined(CONFIG_NAND_SPL) */
u32 DQS_autocalibration(void)
{
	return 0;
}
#endif /* !defined(CONFIG_NAND_U_BOOT) && !defined(CONFIG_NAND_SPL) */

/*
 * arch/powerpc/cpu/ppc4xx/denali_data_eye.c
 * Extracted from board/amcc/sequoia/sdram.c by Larry Johnson <lrj@acm.org>.
 *
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
 *
 * (C) Copyright 2006-2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

/* define DEBUG for debugging output (obviously ;-)) */
#if 0
#define DEBUG
#endif

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/ppc4xx.h>

#if defined(CONFIG_440EPX) || defined(CONFIG_440GRX)
/*-----------------------------------------------------------------------------+
 * denali_wait_for_dlllock.
 +----------------------------------------------------------------------------*/
int denali_wait_for_dlllock(void)
{
	u32 val;
	int wait;

	/* -----------------------------------------------------------+
	 * Wait for the DCC master delay line to finish calibration
	 * ----------------------------------------------------------*/
	for (wait = 0; wait != 0xffff; ++wait) {
		mfsdram(DDR0_17, val);
		if (DDR0_17_DLLLOCKREG_DECODE(val)) {
			/* dlllockreg bit on */
			return 0;
		}
	}
	debug("0x%04x: DDR0_17 Value (dlllockreg bit): 0x%08x\n", wait, val);
	debug("Waiting for dlllockreg bit to raise\n");
	return -1;
}

#if defined(CONFIG_DDR_DATA_EYE)
#define DDR_DCR_BASE 0x10
#define ddrcfga  (DDR_DCR_BASE+0x0)	/* DDR configuration address reg */
#define ddrcfgd  (DDR_DCR_BASE+0x1)	/* DDR configuration data reg    */

/*-----------------------------------------------------------------------------+
 * wait_for_dram_init_complete.
 +----------------------------------------------------------------------------*/
static int wait_for_dram_init_complete(void)
{
	unsigned long val;
	int wait = 0;

	/* --------------------------------------------------------------+
	 * Wait for 'DRAM initialization complete' bit in status register
	 * -------------------------------------------------------------*/
	mtdcr(ddrcfga, DDR0_00);

	while (wait != 0xffff) {
		val = mfdcr(ddrcfgd);
		if ((val & DDR0_00_INT_STATUS_BIT6) == DDR0_00_INT_STATUS_BIT6)
			/* 'DRAM initialization complete' bit */
			return 0;
		else
			wait++;
	}
	debug("DRAM initialization complete bit in status register did not "
	      "rise\n");
	return -1;
}

#define NUM_TRIES 64
#define NUM_READS 10

/*-----------------------------------------------------------------------------+
 * denali_core_search_data_eye.
 +----------------------------------------------------------------------------*/
void denali_core_search_data_eye(void)
{
	int k, j;
	u32 val;
	u32 wr_dqs_shift, dqs_out_shift, dll_dqs_delay_X;
	u32 max_passing_cases = 0, wr_dqs_shift_with_max_passing_cases = 0;
	u32 passing_cases = 0, dll_dqs_delay_X_sw_val = 0;
	u32 dll_dqs_delay_X_start_window = 0, dll_dqs_delay_X_end_window = 0;
	volatile u32 *ram_pointer;
	u32 test[NUM_TRIES] = {
		0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 0x00000000,
		0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		0xAAAAAAAA, 0xAAAAAAAA, 0x55555555, 0x55555555,
		0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		0x55555555, 0x55555555, 0xAAAAAAAA, 0xAAAAAAAA,
		0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		0xA5A5A5A5, 0xA5A5A5A5, 0x5A5A5A5A, 0x5A5A5A5A,
		0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		0x5A5A5A5A, 0x5A5A5A5A, 0xA5A5A5A5, 0xA5A5A5A5,
		0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		0xAA55AA55, 0xAA55AA55, 0x55AA55AA, 0x55AA55AA,
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55,
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55
	};

	ram_pointer = (volatile u32 *)(CONFIG_SYS_SDRAM_BASE);

	for (wr_dqs_shift = 64; wr_dqs_shift < 96; wr_dqs_shift++) {
		/* for (wr_dqs_shift=1; wr_dqs_shift<96; wr_dqs_shift++) { */

		/* -----------------------------------------------------------+
		 * De-assert 'start' parameter.
		 * ----------------------------------------------------------*/
		mtdcr(ddrcfga, DDR0_02);
		val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) |
		    DDR0_02_START_OFF;
		mtdcr(ddrcfgd, val);

		/* -----------------------------------------------------------+
		 * Set 'wr_dqs_shift'
		 * ----------------------------------------------------------*/
		mtdcr(ddrcfga, DDR0_09);
		val = (mfdcr(ddrcfgd) & ~DDR0_09_WR_DQS_SHIFT_MASK) |
		    DDR0_09_WR_DQS_SHIFT_ENCODE(wr_dqs_shift);
		mtdcr(ddrcfgd, val);

		/* -----------------------------------------------------------+
		 * Set 'dqs_out_shift' = wr_dqs_shift + 32
		 * ----------------------------------------------------------*/
		dqs_out_shift = wr_dqs_shift + 32;
		mtdcr(ddrcfga, DDR0_22);
		val = (mfdcr(ddrcfgd) & ~DDR0_22_DQS_OUT_SHIFT_MASK) |
		    DDR0_22_DQS_OUT_SHIFT_ENCODE(dqs_out_shift);
		mtdcr(ddrcfgd, val);

		passing_cases = 0;

		for (dll_dqs_delay_X = 1; dll_dqs_delay_X < 64;
		     dll_dqs_delay_X++) {
			/* for (dll_dqs_delay_X=1; dll_dqs_delay_X<128;
			   dll_dqs_delay_X++) { */
			/* -----------------------------------------------------------+
			 * Set 'dll_dqs_delay_X'.
			 * ----------------------------------------------------------*/
			/* dll_dqs_delay_0 */
			mtdcr(ddrcfga, DDR0_17);
			val = (mfdcr(ddrcfgd) & ~DDR0_17_DLL_DQS_DELAY_0_MASK)
			    | DDR0_17_DLL_DQS_DELAY_0_ENCODE(dll_dqs_delay_X);
			mtdcr(ddrcfgd, val);
			/* dll_dqs_delay_1 to dll_dqs_delay_4 */
			mtdcr(ddrcfga, DDR0_18);
			val = (mfdcr(ddrcfgd) & ~DDR0_18_DLL_DQS_DELAY_X_MASK)
			    | DDR0_18_DLL_DQS_DELAY_4_ENCODE(dll_dqs_delay_X)
			    | DDR0_18_DLL_DQS_DELAY_3_ENCODE(dll_dqs_delay_X)
			    | DDR0_18_DLL_DQS_DELAY_2_ENCODE(dll_dqs_delay_X)
			    | DDR0_18_DLL_DQS_DELAY_1_ENCODE(dll_dqs_delay_X);
			mtdcr(ddrcfgd, val);
			/* dll_dqs_delay_5 to dll_dqs_delay_8 */
			mtdcr(ddrcfga, DDR0_19);
			val = (mfdcr(ddrcfgd) & ~DDR0_19_DLL_DQS_DELAY_X_MASK)
			    | DDR0_19_DLL_DQS_DELAY_8_ENCODE(dll_dqs_delay_X)
			    | DDR0_19_DLL_DQS_DELAY_7_ENCODE(dll_dqs_delay_X)
			    | DDR0_19_DLL_DQS_DELAY_6_ENCODE(dll_dqs_delay_X)
			    | DDR0_19_DLL_DQS_DELAY_5_ENCODE(dll_dqs_delay_X);
			mtdcr(ddrcfgd, val);
			/* clear any ECC errors */
			mtdcr(ddrcfga, DDR0_00);
			mtdcr(ddrcfgd,
			      mfdcr(ddrcfgd) | DDR0_00_INT_ACK_ENCODE(0x3C));

			sync();
			eieio();

			/* -----------------------------------------------------------+
			 * Assert 'start' parameter.
			 * ----------------------------------------------------------*/
			mtdcr(ddrcfga, DDR0_02);
			val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) |
			    DDR0_02_START_ON;
			mtdcr(ddrcfgd, val);

			sync();
			eieio();

			/* -----------------------------------------------------------+
			 * Wait for the DCC master delay line to finish calibration
			 * ----------------------------------------------------------*/
			if (denali_wait_for_dlllock() != 0) {
				printf("dll lock did not occur !!!\n");
				printf("denali_core_search_data_eye!!!\n");
				printf("wr_dqs_shift = %d - dll_dqs_delay_X = "
				       "%d\n", wr_dqs_shift, dll_dqs_delay_X);
				hang();
			}
			sync();
			eieio();

			if (wait_for_dram_init_complete() != 0) {
				printf("dram init complete did not occur!!!\n");
				printf("denali_core_search_data_eye!!!\n");
				printf("wr_dqs_shift = %d - dll_dqs_delay_X = "
				       "%d\n", wr_dqs_shift, dll_dqs_delay_X);
				hang();
			}
			udelay(100); /* wait 100us to ensure init is really completed !!! */

			/* write values */
			for (j = 0; j < NUM_TRIES; j++) {
				ram_pointer[j] = test[j];

				/* clear any cache at ram location */
			      __asm__("dcbf 0,%0": :"r"(&ram_pointer[j]));
			}

			/* read values back */
			for (j = 0; j < NUM_TRIES; j++) {
				for (k = 0; k < NUM_READS; k++) {
					/* clear any cache at ram location */
				      __asm__("dcbf 0,%0": :"r"(&ram_pointer
					    [j]));

					if (ram_pointer[j] != test[j])
						break;
				}

				/* read error */
				if (k != NUM_READS)
					break;
			}

			/* See if the dll_dqs_delay_X value passed. */
			mtdcr(ddrcfga, DDR0_00);
			if (j < NUM_TRIES
			    || (DDR0_00_INT_STATUS_DECODE(mfdcr(ddrcfgd)) &
				0x3F)) {
				/* Failed */
				passing_cases = 0;
				/* break; */
			} else {
				/* Passed */
				if (passing_cases == 0)
					dll_dqs_delay_X_sw_val =
					    dll_dqs_delay_X;
				passing_cases++;
				if (passing_cases >= max_passing_cases) {
					max_passing_cases = passing_cases;
					wr_dqs_shift_with_max_passing_cases =
					    wr_dqs_shift;
					dll_dqs_delay_X_start_window =
					    dll_dqs_delay_X_sw_val;
					dll_dqs_delay_X_end_window =
					    dll_dqs_delay_X;
				}
			}

			/* -----------------------------------------------------------+
			 * De-assert 'start' parameter.
			 * ----------------------------------------------------------*/
			mtdcr(ddrcfga, DDR0_02);
			val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) |
			    DDR0_02_START_OFF;
			mtdcr(ddrcfgd, val);
		} /* for (dll_dqs_delay_X=0; dll_dqs_delay_X<128; dll_dqs_delay_X++) */
	} /* for (wr_dqs_shift=0; wr_dqs_shift<96; wr_dqs_shift++) */

	/* -----------------------------------------------------------+
	 * Largest passing window is now detected.
	 * ----------------------------------------------------------*/

	/* Compute dll_dqs_delay_X value */
	dll_dqs_delay_X = (dll_dqs_delay_X_end_window +
			   dll_dqs_delay_X_start_window) / 2;
	wr_dqs_shift = wr_dqs_shift_with_max_passing_cases;

	debug("DQS calibration - Window detected:\n");
	debug("max_passing_cases = %d\n", max_passing_cases);
	debug("wr_dqs_shift      = %d\n", wr_dqs_shift);
	debug("dll_dqs_delay_X   = %d\n", dll_dqs_delay_X);
	debug("dll_dqs_delay_X window = %d - %d\n",
	      dll_dqs_delay_X_start_window, dll_dqs_delay_X_end_window);

	/* -----------------------------------------------------------+
	 * De-assert 'start' parameter.
	 * ----------------------------------------------------------*/
	mtdcr(ddrcfga, DDR0_02);
	val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_OFF;
	mtdcr(ddrcfgd, val);

	/* -----------------------------------------------------------+
	 * Set 'wr_dqs_shift'
	 * ----------------------------------------------------------*/
	mtdcr(ddrcfga, DDR0_09);
	val = (mfdcr(ddrcfgd) & ~DDR0_09_WR_DQS_SHIFT_MASK)
	    | DDR0_09_WR_DQS_SHIFT_ENCODE(wr_dqs_shift);
	mtdcr(ddrcfgd, val);
	debug("DDR0_09=0x%08lx\n", val);

	/* -----------------------------------------------------------+
	 * Set 'dqs_out_shift' = wr_dqs_shift + 32
	 * ----------------------------------------------------------*/
	dqs_out_shift = wr_dqs_shift + 32;
	mtdcr(ddrcfga, DDR0_22);
	val = (mfdcr(ddrcfgd) & ~DDR0_22_DQS_OUT_SHIFT_MASK)
	    | DDR0_22_DQS_OUT_SHIFT_ENCODE(dqs_out_shift);
	mtdcr(ddrcfgd, val);
	debug("DDR0_22=0x%08lx\n", val);

	/* -----------------------------------------------------------+
	 * Set 'dll_dqs_delay_X'.
	 * ----------------------------------------------------------*/
	/* dll_dqs_delay_0 */
	mtdcr(ddrcfga, DDR0_17);
	val = (mfdcr(ddrcfgd) & ~DDR0_17_DLL_DQS_DELAY_0_MASK)
	    | DDR0_17_DLL_DQS_DELAY_0_ENCODE(dll_dqs_delay_X);
	mtdcr(ddrcfgd, val);
	debug("DDR0_17=0x%08lx\n", val);

	/* dll_dqs_delay_1 to dll_dqs_delay_4 */
	mtdcr(ddrcfga, DDR0_18);
	val = (mfdcr(ddrcfgd) & ~DDR0_18_DLL_DQS_DELAY_X_MASK)
	    | DDR0_18_DLL_DQS_DELAY_4_ENCODE(dll_dqs_delay_X)
	    | DDR0_18_DLL_DQS_DELAY_3_ENCODE(dll_dqs_delay_X)
	    | DDR0_18_DLL_DQS_DELAY_2_ENCODE(dll_dqs_delay_X)
	    | DDR0_18_DLL_DQS_DELAY_1_ENCODE(dll_dqs_delay_X);
	mtdcr(ddrcfgd, val);
	debug("DDR0_18=0x%08lx\n", val);

	/* dll_dqs_delay_5 to dll_dqs_delay_8 */
	mtdcr(ddrcfga, DDR0_19);
	val = (mfdcr(ddrcfgd) & ~DDR0_19_DLL_DQS_DELAY_X_MASK)
	    | DDR0_19_DLL_DQS_DELAY_8_ENCODE(dll_dqs_delay_X)
	    | DDR0_19_DLL_DQS_DELAY_7_ENCODE(dll_dqs_delay_X)
	    | DDR0_19_DLL_DQS_DELAY_6_ENCODE(dll_dqs_delay_X)
	    | DDR0_19_DLL_DQS_DELAY_5_ENCODE(dll_dqs_delay_X);
	mtdcr(ddrcfgd, val);
	debug("DDR0_19=0x%08lx\n", val);

	/* -----------------------------------------------------------+
	 * Assert 'start' parameter.
	 * ----------------------------------------------------------*/
	mtdcr(ddrcfga, DDR0_02);
	val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_ON;
	mtdcr(ddrcfgd, val);

	sync();
	eieio();

	/* -----------------------------------------------------------+
	 * Wait for the DCC master delay line to finish calibration
	 * ----------------------------------------------------------*/
	if (denali_wait_for_dlllock() != 0) {
		printf("dll lock did not occur !!!\n");
		hang();
	}
	sync();
	eieio();

	if (wait_for_dram_init_complete() != 0) {
		printf("dram init complete did not occur !!!\n");
		hang();
	}
	udelay(100); /* wait 100us to ensure init is really completed !!! */
}
#endif /* defined(CONFIG_DDR_DATA_EYE) */
#endif /* defined(CONFIG_440EPX) || defined(CONFIG_440GRX) */

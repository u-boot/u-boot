/*
 * (C) Copyright 2006
 * Sylvie Gohl,             AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,           AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,            AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,           AMCC/IBM, rob.snyder@fr.ibm.com
 *
 * (C) Copyright 2007
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
#include <asm/mmu.h>
#include <asm/io.h>
#include <ppc440.h>

#include "sdram.h"

/*
 * This DDR2 setup code can dynamically setup the TLB entries for the DDR2 memory
 * region. Right now the cache should still be disabled in U-Boot because of the
 * EMAC driver, that need it's buffer descriptor to be located in non cached
 * memory.
 *
 * If at some time this restriction doesn't apply anymore, just define
 * CFG_ENABLE_SDRAM_CACHE in the board config file and this code should setup
 * everything correctly.
 */
#ifdef CFG_ENABLE_SDRAM_CACHE
#define MY_TLB_WORD2_I_ENABLE   0                       /* enable caching on SDRAM */
#else
#define MY_TLB_WORD2_I_ENABLE   TLB_WORD2_I_ENABLE      /* disable caching on SDRAM */
#endif

void program_tlb(u32 phys_addr, u32 virt_addr, u32 size, u32 tlb_word2_i_value);
void dcbz_area(u32 start_address, u32 num_bytes);
void dflush(void);

#ifdef CONFIG_ADD_RAM_INFO
static u32 is_ecc_enabled(void)
{
	u32 val;

	mfsdram(DDR0_22, val);
	val &= DDR0_22_CTRL_RAW_MASK;
	if (val)
		return 1;
	else
		return 0;
}

void board_add_ram_info(int use_default)
{
	PPC440_SYS_INFO board_cfg;
	u32 val;

	if (is_ecc_enabled())
		puts(" (ECC");
	else
		puts(" (ECC not");

	get_sys_info(&board_cfg);
	printf(" enabled, %d MHz", (board_cfg.freqPLB * 2) / 1000000);

	mfsdram(DDR0_03, val);
	val = DDR0_03_CASLAT_DECODE(val);
	printf(", CL%d)", val);
}
#endif

static int wait_for_dlllock(void)
{
	u32 val;
	int wait = 0;

	/*
	 * Wait for the DCC master delay line to finish calibration
	 */
	mtdcr(ddrcfga, DDR0_17);
	val = DDR0_17_DLLLOCKREG_UNLOCKED;

	while (wait != 0xffff) {
		val = mfdcr(ddrcfgd);
		if ((val & DDR0_17_DLLLOCKREG_MASK) == DDR0_17_DLLLOCKREG_LOCKED)
			/* dlllockreg bit on */
			return 0;
		else
			wait++;
	}
	debug("0x%04x: DDR0_17 Value (dlllockreg bit): 0x%08x\n", wait, val);
	debug("Waiting for dlllockreg bit to raise\n");

	return -1;
}

#if defined(CONFIG_DDR_DATA_EYE)
int wait_for_dram_init_complete(void)
{
	u32 val;
	int wait = 0;

	/*
	 * Wait for 'DRAM initialization complete' bit in status register
	 */
	mtdcr(ddrcfga, DDR0_00);

	while (wait != 0xffff) {
		val = mfdcr(ddrcfgd);
		if ((val & DDR0_00_INT_STATUS_BIT6) == DDR0_00_INT_STATUS_BIT6)
			/* 'DRAM initialization complete' bit */
			return 0;
		else
			wait++;
	}

	debug("DRAM initialization complete bit in status register did not rise\n");

	return -1;
}

#define NUM_TRIES 64
#define NUM_READS 10

void denali_core_search_data_eye(u32 start_addr, u32 memory_size)
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
		0x55AA55AA, 0x55AA55AA, 0xAA55AA55, 0xAA55AA55 };

	ram_pointer = (volatile u32 *)start_addr;

	for (wr_dqs_shift = 64; wr_dqs_shift < 96; wr_dqs_shift++) {
		/*for (wr_dqs_shift=1; wr_dqs_shift<96; wr_dqs_shift++) {*/

		/*
		 * De-assert 'start' parameter.
		 */
		mtdcr(ddrcfga, DDR0_02);
		val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_OFF;
		mtdcr(ddrcfgd, val);

		/*
		 * Set 'wr_dqs_shift'
		 */
		mtdcr(ddrcfga, DDR0_09);
		val = (mfdcr(ddrcfgd) & ~DDR0_09_WR_DQS_SHIFT_MASK)
			| DDR0_09_WR_DQS_SHIFT_ENCODE(wr_dqs_shift);
		mtdcr(ddrcfgd, val);

		/*
		 * Set 'dqs_out_shift' = wr_dqs_shift + 32
		 */
		dqs_out_shift = wr_dqs_shift + 32;
		mtdcr(ddrcfga, DDR0_22);
		val = (mfdcr(ddrcfgd) & ~DDR0_22_DQS_OUT_SHIFT_MASK)
			| DDR0_22_DQS_OUT_SHIFT_ENCODE(dqs_out_shift);
		mtdcr(ddrcfgd, val);

		passing_cases = 0;

		for (dll_dqs_delay_X = 1; dll_dqs_delay_X < 64; dll_dqs_delay_X++) {
			/*for (dll_dqs_delay_X=1; dll_dqs_delay_X<128; dll_dqs_delay_X++) {*/
			/*
			 * Set 'dll_dqs_delay_X'.
			 */
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

			ppcMsync();
			ppcMbar();

			/*
			 * Assert 'start' parameter.
			 */
			mtdcr(ddrcfga, DDR0_02);
			val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_ON;
			mtdcr(ddrcfgd, val);

			ppcMsync();
			ppcMbar();

			/*
			 * Wait for the DCC master delay line to finish calibration
			 */
			if (wait_for_dlllock() != 0) {
				printf("dlllock did not occur !!!\n");
				printf("denali_core_search_data_eye!!!\n");
				printf("wr_dqs_shift = %d - dll_dqs_delay_X = %d\n",
				       wr_dqs_shift, dll_dqs_delay_X);
				hang();
			}
			ppcMsync();
			ppcMbar();

			if (wait_for_dram_init_complete() != 0) {
				printf("dram init complete did not occur !!!\n");
				printf("denali_core_search_data_eye!!!\n");
				printf("wr_dqs_shift = %d - dll_dqs_delay_X = %d\n",
				       wr_dqs_shift, dll_dqs_delay_X);
				hang();
			}
			udelay(100);  /* wait 100us to ensure init is really completed !!! */

			/* write values */
			for (j=0; j<NUM_TRIES; j++) {
				ram_pointer[j] = test[j];

				/* clear any cache at ram location */
				__asm__("dcbf 0,%0": :"r" (&ram_pointer[j]));
			}

			/* read values back */
			for (j=0; j<NUM_TRIES; j++) {
				for (k=0; k<NUM_READS; k++) {
					/* clear any cache at ram location */
					__asm__("dcbf 0,%0": :"r" (&ram_pointer[j]));

					if (ram_pointer[j] != test[j])
						break;
				}

				/* read error */
				if (k != NUM_READS)
					break;
			}

			/* See if the dll_dqs_delay_X value passed.*/
			if (j < NUM_TRIES) {
				/* Failed */
				passing_cases = 0;
				/* break; */
			} else {
				/* Passed */
				if (passing_cases == 0)
					dll_dqs_delay_X_sw_val = dll_dqs_delay_X;
				passing_cases++;
				if (passing_cases >= max_passing_cases) {
					max_passing_cases = passing_cases;
					wr_dqs_shift_with_max_passing_cases = wr_dqs_shift;
					dll_dqs_delay_X_start_window = dll_dqs_delay_X_sw_val;
					dll_dqs_delay_X_end_window = dll_dqs_delay_X;
				}
			}

			/*
			 * De-assert 'start' parameter.
			 */
			mtdcr(ddrcfga, DDR0_02);
			val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_OFF;
			mtdcr(ddrcfgd, val);

		} /* for (dll_dqs_delay_X=0; dll_dqs_delay_X<128; dll_dqs_delay_X++) */

	} /* for (wr_dqs_shift=0; wr_dqs_shift<96; wr_dqs_shift++) */

	/*
	 * Largest passing window is now detected.
	 */

	/* Compute dll_dqs_delay_X value */
	dll_dqs_delay_X = (dll_dqs_delay_X_end_window + dll_dqs_delay_X_start_window) / 2;
	wr_dqs_shift = wr_dqs_shift_with_max_passing_cases;

	debug("DQS calibration - Window detected:\n");
	debug("max_passing_cases = %d\n", max_passing_cases);
	debug("wr_dqs_shift      = %d\n", wr_dqs_shift);
	debug("dll_dqs_delay_X   = %d\n", dll_dqs_delay_X);
	debug("dll_dqs_delay_X window = %d - %d\n",
	      dll_dqs_delay_X_start_window, dll_dqs_delay_X_end_window);

	/*
	 * De-assert 'start' parameter.
	 */
	mtdcr(ddrcfga, DDR0_02);
	val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_OFF;
	mtdcr(ddrcfgd, val);

	/*
	 * Set 'wr_dqs_shift'
	 */
	mtdcr(ddrcfga, DDR0_09);
	val = (mfdcr(ddrcfgd) & ~DDR0_09_WR_DQS_SHIFT_MASK)
		| DDR0_09_WR_DQS_SHIFT_ENCODE(wr_dqs_shift);
	mtdcr(ddrcfgd, val);
	debug("DDR0_09=0x%08lx\n", val);

	/*
	 * Set 'dqs_out_shift' = wr_dqs_shift + 32
	 */
	dqs_out_shift = wr_dqs_shift + 32;
	mtdcr(ddrcfga, DDR0_22);
	val = (mfdcr(ddrcfgd) & ~DDR0_22_DQS_OUT_SHIFT_MASK)
		| DDR0_22_DQS_OUT_SHIFT_ENCODE(dqs_out_shift);
	mtdcr(ddrcfgd, val);
	debug("DDR0_22=0x%08lx\n", val);

	/*
	 * Set 'dll_dqs_delay_X'.
	 */
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

	/*
	 * Assert 'start' parameter.
	 */
	mtdcr(ddrcfga, DDR0_02);
	val = (mfdcr(ddrcfgd) & ~DDR0_02_START_MASK) | DDR0_02_START_ON;
	mtdcr(ddrcfgd, val);

	ppcMsync();
	ppcMbar();

	/*
	 * Wait for the DCC master delay line to finish calibration
	 */
	if (wait_for_dlllock() != 0) {
		printf("dlllock did not occur !!!\n");
		hang();
	}
	ppcMsync();
	ppcMbar();

	if (wait_for_dram_init_complete() != 0) {
		printf("dram init complete did not occur !!!\n");
		hang();
	}
	udelay(100);  /* wait 100us to ensure init is really completed !!! */
}
#endif /* CONFIG_DDR_DATA_EYE */

#ifdef CONFIG_DDR_ECC
static void wait_ddr_idle(void)
{
	/*
	 * Controller idle status cannot be determined for Denali
	 * DDR2 code. Just return here.
	 */
}

static void blank_string(int size)
{
	int i;

	for (i=0; i<size; i++)
		putc('\b');
	for (i=0; i<size; i++)
		putc(' ');
	for (i=0; i<size; i++)
		putc('\b');
}

static void program_ecc(u32 start_address,
			u32 num_bytes,
			u32 tlb_word2_i_value)
{
	u32 current_address;
	u32 end_address;
	u32 address_increment;
	u32 val;
	char str[] = "ECC generation -";
	char slash[] = "\\|/-\\|/-";
	int loop = 0;
	int loopi = 0;

	current_address = start_address;

	sync();
	eieio();
	wait_ddr_idle();

	if (tlb_word2_i_value == TLB_WORD2_I_ENABLE) {
		/* ECC bit set method for non-cached memory */
		address_increment = 4;
		end_address = current_address + num_bytes;

		puts(str);

		while (current_address < end_address) {
			*((u32 *)current_address) = 0x00000000;
			current_address += address_increment;

			if ((loop++ % (2 << 20)) == 0) {
				putc('\b');
				putc(slash[loopi++ % 8]);
			}
		}

		blank_string(strlen(str));
	} else {
		/* ECC bit set method for cached memory */
		dcbz_area(start_address, num_bytes);
		dflush();
	}

	sync();
	eieio();
	wait_ddr_idle();

	/* Clear error status */
	mfsdram(DDR0_00, val);
	mtsdram(DDR0_00, val | DDR0_00_INT_ACK_ALL);

	/* Set 'int_mask' parameter to functionnal value */
	mfsdram(DDR0_01, val);
	mtsdram(DDR0_01, ((val &~ DDR0_01_INT_MASK_MASK) | DDR0_01_INT_MASK_ALL_OFF));

	sync();
	eieio();
	wait_ddr_idle();
}
#endif

static __inline__ u32 get_mcsr(void)
{
	u32 val;

	asm volatile("mfspr %0, 0x23c" : "=r" (val) :);
	return val;
}

static __inline__ void set_mcsr(u32 val)
{
	asm volatile("mtspr 0x23c, %0" : "=r" (val) :);
}

/*************************************************************************
 *
 * initdram -- 440EPx's DDR controller is a DENALI Core
 *
 ************************************************************************/
long int initdram (int board_type)
{
	u32 val;

	mtsdram(DDR0_02, 0x00000000);

	mtsdram(DDR0_00, 0x0000190A);
	mtsdram(DDR0_01, 0x01000000);
	mtsdram(DDR0_03, 0x02030603); /* A suitable burst length was taken. CAS is right for our board */

	mtsdram(DDR0_04, 0x0A030300);
	mtsdram(DDR0_05, 0x02020308);
	mtsdram(DDR0_06, 0x0103C812);
	mtsdram(DDR0_07, 0x00090100);
	mtsdram(DDR0_08, 0x02c80001);
	mtsdram(DDR0_09, 0x00011D5F);
	mtsdram(DDR0_10, 0x00000300);
	mtsdram(DDR0_11, 0x000CC800);
	mtsdram(DDR0_12, 0x00000003);
	mtsdram(DDR0_14, 0x00000000);
	mtsdram(DDR0_17, 0x1e000000);
	mtsdram(DDR0_18, 0x1e1e1e1e);
	mtsdram(DDR0_19, 0x1e1e1e1e);
	mtsdram(DDR0_20, 0x0B0B0B0B);
	mtsdram(DDR0_21, 0x0B0B0B0B);
#ifdef CONFIG_DDR_ECC
	mtsdram(DDR0_22, 0x00267F0B | DDR0_22_CTRL_RAW_ECC_ENABLE); /* enable ECC	*/
#else
	mtsdram(DDR0_22, 0x00267F0B);
#endif

	mtsdram(DDR0_23, 0x01000000);
	mtsdram(DDR0_24, 0x01010001);

	mtsdram(DDR0_26, 0x2D93028A);
	mtsdram(DDR0_27, 0x0784682B);

	mtsdram(DDR0_28, 0x00000080);
	mtsdram(DDR0_31, 0x00000000);
	mtsdram(DDR0_42, 0x01000006);

	mtsdram(DDR0_43, 0x030A0200);
	mtsdram(DDR0_44, 0x00000003);
	mtsdram(DDR0_02, 0x00000001); /* Activate the denali core */

	wait_for_dlllock();

        /*
	 * Program tlb entries for this size (dynamic)
	 */
        program_tlb(0, 0, CFG_MBYTES_SDRAM << 20, MY_TLB_WORD2_I_ENABLE);

	/*
	 * Setup 2nd TLB with same physical address but different virtual address
	 * with cache enabled. This is done for fast ECC generation.
	 */
        program_tlb(0, CFG_DDR_CACHED_ADDR, CFG_MBYTES_SDRAM << 20, 0);

#ifdef CONFIG_DDR_DATA_EYE
	/*
	 * Perform data eye search if requested.
	 */
	denali_core_search_data_eye(CFG_DDR_CACHED_ADDR, CFG_MBYTES_SDRAM << 20);

	/*
	 * Clear possible errors resulting from data-eye-search.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	val = get_mcsr();
	set_mcsr(val);
#endif

#ifdef CONFIG_DDR_ECC
	/*
	 * If ECC is enabled, initialize the parity bits.
	 */
	program_ecc(CFG_DDR_CACHED_ADDR, CFG_MBYTES_SDRAM << 20, 0);
#endif

	return (CFG_MBYTES_SDRAM << 20);
}

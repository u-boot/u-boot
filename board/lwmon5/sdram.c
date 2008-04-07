/*
 * (C) Copyright 2006
 * Sylvie Gohl,		    AMCC/IBM, gohl.sylvie@fr.ibm.com
 * Jacqueline Pira-Ferriol, AMCC/IBM, jpira-ferriol@fr.ibm.com
 * Thierry Roman,	    AMCC/IBM, thierry_roman@fr.ibm.com
 * Alain Saurel,	    AMCC/IBM, alain.saurel@fr.ibm.com
 * Robert Snyder,	    AMCC/IBM, rob.snyder@fr.ibm.com
 *
 * (C) Copyright 2007-2008
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
#include <watchdog.h>

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
#define MY_TLB_WORD2_I_ENABLE	0			/* enable caching on SDRAM */
#else
#define MY_TLB_WORD2_I_ENABLE	TLB_WORD2_I_ENABLE	/* disable caching on SDRAM */
#endif

/*-----------------------------------------------------------------------------+
 * Prototypes
 *-----------------------------------------------------------------------------*/
extern int denali_wait_for_dlllock(void);
extern void denali_core_search_data_eye(void);
extern void dcbz_area(u32 start_address, u32 num_bytes);
extern void dflush(void);

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
	PPC4xx_SYS_INFO board_cfg;
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

#ifdef CONFIG_DDR_ECC
static void wait_ddr_idle(void)
{
	/*
	 * Controller idle status cannot be determined for Denali
	 * DDR2 code. Just return here.
	 */
}

static void program_ecc(u32 start_address,
			u32 num_bytes,
			u32 tlb_word2_i_value)
{
	u32 val;
	u32 current_addr = start_address;
	int bytes_remaining;

	sync();
	wait_ddr_idle();

	/*
	 * Because of 440EPx errata CHIP 11, we don't touch the last 256
	 * bytes of SDRAM.
	 */
	bytes_remaining = num_bytes - CFG_MEM_TOP_HIDE;

	/*
	 * We have to write the ECC bytes by zeroing and flushing in smaller
	 * steps, since the whole 256MByte takes too long for the external
	 * watchdog.
	 */
	while (bytes_remaining > 0) {
		dcbz_area(current_addr, min((64 << 20), bytes_remaining));
		current_addr += 64 << 20;
		bytes_remaining -= 64 << 20;
		WATCHDOG_RESET();
	}
	dflush();

	sync();
	wait_ddr_idle();

	/* Clear error status */
	mfsdram(DDR0_00, val);
	mtsdram(DDR0_00, val | DDR0_00_INT_ACK_ALL);

	/* Set 'int_mask' parameter to functionnal value */
	mfsdram(DDR0_01, val);
	mtsdram(DDR0_01, ((val &~ DDR0_01_INT_MASK_MASK) | DDR0_01_INT_MASK_ALL_OFF));

	sync();
	wait_ddr_idle();
}
#endif

/*************************************************************************
 *
 * initdram -- 440EPx's DDR controller is a DENALI Core
 *
 ************************************************************************/
long int initdram (int board_type)
{
#if 0 /* test-only: will remove this define later, when ECC problems are solved! */
	/* CL=3 */
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
#else
	/* CL=4 */
	mtsdram(DDR0_02, 0x00000000);

	mtsdram(DDR0_00, 0x0000190A);
	mtsdram(DDR0_01, 0x01000000);
	mtsdram(DDR0_03, 0x02040803); /* A suitable burst length was taken. CAS is right for our board */

	mtsdram(DDR0_04, 0x0B030300);
	mtsdram(DDR0_05, 0x02020308);
	mtsdram(DDR0_06, 0x0003C812);
	mtsdram(DDR0_07, 0x00090100);
	mtsdram(DDR0_08, 0x03c80001);
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
	mtsdram(DDR0_22, 0x00267F0B | DDR0_22_CTRL_RAW_ECC_ENABLE); /* enable ECC       */
#else
	mtsdram(DDR0_22, 0x00267F0B);
#endif

	mtsdram(DDR0_23, 0x01000000);
	mtsdram(DDR0_24, 0x01010001);

	mtsdram(DDR0_26, 0x2D93028A);
	mtsdram(DDR0_27, 0x0784682B);

	mtsdram(DDR0_28, 0x00000080);
	mtsdram(DDR0_31, 0x00000000);
	mtsdram(DDR0_42, 0x01000008);

	mtsdram(DDR0_43, 0x050A0200);
	mtsdram(DDR0_44, 0x00000005);
	mtsdram(DDR0_02, 0x00000001); /* Activate the denali core */
#endif

	denali_wait_for_dlllock();

#if defined(CONFIG_DDR_DATA_EYE)
	/* -----------------------------------------------------------+
	 * Perform data eye search if requested.
	 * ----------------------------------------------------------*/
	program_tlb(0, CFG_SDRAM_BASE, CFG_MBYTES_SDRAM << 20,
		    TLB_WORD2_I_ENABLE);
	denali_core_search_data_eye();
	remove_tlb(CFG_SDRAM_BASE, CFG_MBYTES_SDRAM << 20);
#endif

	/*
	 * Program tlb entries for this size (dynamic)
	 */
	program_tlb(0, CFG_SDRAM_BASE, CFG_MBYTES_SDRAM << 20,
		    MY_TLB_WORD2_I_ENABLE);

	/*
	 * Setup 2nd TLB with same physical address but different virtual address
	 * with cache enabled. This is done for fast ECC generation.
	 */
	program_tlb(0, CFG_DDR_CACHED_ADDR, CFG_MBYTES_SDRAM << 20, 0);

#ifdef CONFIG_DDR_ECC
	/*
	 * If ECC is enabled, initialize the parity bits.
	 */
	program_ecc(CFG_DDR_CACHED_ADDR, CFG_MBYTES_SDRAM << 20, 0);
#endif

	/*
	 * Clear possible errors resulting from data-eye-search.
	 * If not done, then we could get an interrupt later on when
	 * exceptions are enabled.
	 */
	set_mcsr(get_mcsr());

	return (CFG_MBYTES_SDRAM << 20);
}

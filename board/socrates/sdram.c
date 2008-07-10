/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.         See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */


#include <common.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <spd_sdram.h>


#if !defined(CONFIG_SPD_EEPROM)
/*
 * Autodetect onboard DDR SDRAM on 85xx platforms
 *
 * NOTE: Some of the hardcoded values are hardware dependant,
 *       so this should be extended for other future boards
 *       using this routine!
 */
long int sdram_setup(int casl)
{
	volatile ccsr_ddr_t *ddr = (void *)(CFG_MPC85xx_DDR_ADDR);

	/*
	 * Disable memory controller.
	 */
	ddr->cs0_config = 0;
	ddr->sdram_cfg = 0;

	ddr->cs0_bnds = CFG_DDR_CS0_BNDS;
	ddr->cs0_config = CFG_DDR_CS0_CONFIG;
	ddr->timing_cfg_0 = CFG_DDR_TIMING_0;
	ddr->timing_cfg_1 = CFG_DDR_TIMING_1;
	ddr->timing_cfg_2 = CFG_DDR_TIMING_2;
	ddr->sdram_mode = CFG_DDR_MODE;
	ddr->sdram_interval = CFG_DDR_INTERVAL;
	ddr->sdram_cfg_2 = CFG_DDR_CONFIG_2;
	ddr->sdram_clk_cntl = CFG_DDR_CLK_CONTROL;

	asm ("sync;isync;msync");
	udelay(1000);

	ddr->sdram_cfg = CFG_DDR_CONFIG;
	asm ("sync; isync; msync");
	udelay(1000);

	if (get_ram_size(0, CFG_SDRAM_SIZE<<20) == CFG_SDRAM_SIZE<<20) {
		/*
		 * OK, size detected -> all done
		 */
		return CFG_SDRAM_SIZE<<20;
	}

	return 0;				/* nothing found !		*/
}
#endif

phys_size_t initdram (int board_type)
{
	long dram_size = 0;
#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = sdram_setup(CONFIG_DDR_DEFAULT_CL);
#endif
	return dram_size;
}

#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf ("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf ("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf ("SDRAM test passed.\n");
	return 0;
}
#endif

 /*
 * Copyright 2004 Freescale Semiconductor.
 * (C) Copyright 2002,2003, Motorola Inc.
 * Xianghua Xiao, (X.Xiao@motorola.com)
 *
 * (C) Copyright 2002 Scott McNutt <smcnutt@artesyncp.com>
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
 */


#include <common.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <spd.h>

#if defined(CONFIG_DDR_ECC)
extern void ddr_enable_ecc(unsigned int dram_size);
#endif

extern long int spd_sdram(void);

void sdram_init(void);
long int fixed_sdram(void);


int board_early_init_f (void)
{
#if defined(CONFIG_PCI)
	volatile immap_t *immr = (immap_t *) CFG_IMMR;
	volatile ccsr_pcix_t *pci = &immr->im_pcix;

	pci->peer &= 0xffffffdf;	/* disable master abort */
#endif

	return 0;
}

int checkboard (void)
{
	puts("Board: ADS\n");

#ifdef CONFIG_PCI
	printf("    PCI1: 32 bit, %d MHz (compiled)\n",
	       CONFIG_SYS_CLK_FREQ / 1000000);
#else
	printf("    PCI1: disabled\n");
#endif

	return 0;
}


long int
initdram(int board_type)
{
	long dram_size = 0;
	extern long spd_sdram (void);
	volatile immap_t *immap = (immap_t *)CFG_IMMR;

	puts("Initializing\n");

#if defined(CONFIG_DDR_DLL)
	{
		volatile ccsr_gur_t *gur= &immap->im_gur;
		uint temp_ddrdll = 0;

		/*
		* Work around to stabilize DDR DLL
		*/
		temp_ddrdll = gur->ddrdllcr;
		gur->ddrdllcr = ((temp_ddrdll & 0xff) << 16) | 0x80000000;
		asm("sync;isync;msync");
	}
#endif

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif

#if defined(CONFIG_DDR_ECC)
	/*
	 * Initialize and enable DDR ECC.
	 */
	ddr_enable_ecc(dram_size);
#endif

	/*
	 * Initialize SDRAM.
	 */
	sdram_init();

	puts("    DDR: ");
	return dram_size;
}


/*
 * Initialize SDRAM memory on the Local Bus.
 */

void sdram_init (void)
{
#if !defined(CONFIG_RAM_AS_FLASH)
	sys_info_t sysinfo;
	volatile immap_t *immap = (immap_t *) CFG_IMMR;
	volatile ccsr_lbc_t *lbc = &immap->im_lbc;
	uint *sdram_addr = (uint *) CFG_LBC_SDRAM_BASE;

	puts ("    SDRAM: ");
	print_size (CFG_LBC_SDRAM_SIZE * 1024 * 1024, "\n");

	/*
	 * LocalBus SDRAM is not emulating flash.
	 */

	/*
	 * Fix Local Bus clock glitch.  Errata LBC11.
	 *
	 * If localbus freq is less than 66Mhz, use bypass mode,
	 * otherwise use DLL.
	 * lcrr is the local-bus clock ratio register.
	 */
	get_sys_info (&sysinfo);
	if (sysinfo.freqSystemBus / (CFG_LBC_LCRR & 0x0f) < 66000000) {
		lbc->lcrr = (CFG_LBC_LCRR & 0x0fffffff) | 0x80000000;

	} else {
		/*
		 * On REV1 boards, need to change CLKDIV before enable DLL.
		 * Default CLKDIV is 8, change it to 4 temporarily.
		 */
		volatile ccsr_gur_t *gur = &immap->im_gur;
		uint pvr = get_pvr ();
		uint temp_lbcdll = 0;

		if (pvr == PVR_85xx_REV1) {
			lbc->lcrr = 0x10000004;
		}

		/* FIXME: jdl  Should lcrr have 0x8000000 OR'ed in here too? */
		lbc->lcrr = CFG_LBC_LCRR & 0x7fffffff;
		udelay (200);
		temp_lbcdll = gur->lbcdllcr;
		gur->lbcdllcr = ((temp_lbcdll & 0xff) << 16) | 0x80000000;
		asm ("sync;isync;msync");
	}

	/*
	 * Setup SDRAM Base and Option Registers
	 */
	lbc->or2 = CFG_OR2_PRELIM;
	lbc->br2 = CFG_BR2_PRELIM;
	lbc->lbcr = CFG_LBC_LBCR;
	asm ("msync");

	lbc->lsrt = CFG_LBC_LSRT;
	lbc->mrtpr = CFG_LBC_MRTPR;
	asm ("sync");

	/*
	 * Configure the SDRAM controller.
	 */
	lbc->lsdmr = CFG_LBC_LSDMR_1;
	asm ("sync");
	*sdram_addr = 0xff;
	ppcDcbf ((unsigned long) sdram_addr);
	udelay (100);

	lbc->lsdmr = CFG_LBC_LSDMR_2;
	asm ("sync");
	*sdram_addr = 0xff;
	ppcDcbf ((unsigned long) sdram_addr);
	udelay (100);

	lbc->lsdmr = CFG_LBC_LSDMR_3;
	asm ("sync");
	*sdram_addr = 0xff;
	ppcDcbf ((unsigned long) sdram_addr);
	udelay (100);

	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm ("sync");
	*sdram_addr = 0xff;
	ppcDcbf ((unsigned long) sdram_addr);
	udelay (100);

	lbc->lsdmr = CFG_LBC_LSDMR_5;
	asm ("sync");
	*sdram_addr = 0xff;
	ppcDcbf ((unsigned long) sdram_addr);
	udelay (100);

#endif
}


#if defined(CFG_DRAM_TEST)
int testdram (void)
{
	uint *pstart = (uint *) CFG_MEMTEST_START;
	uint *pend = (uint *) CFG_MEMTEST_END;
	uint *p;

	printf("SDRAM test phase 1:\n");
	for (p = pstart; p < pend; p++)
		*p = 0xaaaaaaaa;

	for (p = pstart; p < pend; p++) {
		if (*p != 0xaaaaaaaa) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test phase 2:\n");
	for (p = pstart; p < pend; p++)
		*p = 0x55555555;

	for (p = pstart; p < pend; p++) {
		if (*p != 0x55555555) {
			printf ("SDRAM test fails at: %08x\n", (uint) p);
			return 1;
		}
	}

	printf("SDRAM test passed.\n");
	return 0;
}
#endif


#if !defined(CONFIG_SPD_EEPROM)
/*************************************************************************
 *  fixed sdram init -- doesn't use serial presence detect.
 ************************************************************************/
long int fixed_sdram (void)
{
  #ifndef CFG_RAMBOOT
	volatile immap_t *immap = (immap_t *)CFG_IMMR;
	volatile ccsr_ddr_t *ddr= &immap->im_ddr;

	ddr->cs0_bnds = CFG_DDR_CS0_BNDS;
	ddr->cs0_config = CFG_DDR_CS0_CONFIG;
	ddr->timing_cfg_1 = CFG_DDR_TIMING_1;
	ddr->timing_cfg_2 = CFG_DDR_TIMING_2;
	ddr->sdram_mode = CFG_DDR_MODE;
	ddr->sdram_interval = CFG_DDR_INTERVAL;
    #if defined (CONFIG_DDR_ECC)
	ddr->err_disable = 0x0000000D;
	ddr->err_sbe = 0x00ff0000;
    #endif
	asm("sync;isync;msync");
	udelay(500);
    #if defined (CONFIG_DDR_ECC)
	/* Enable ECC checking */
	ddr->sdram_cfg = (CFG_DDR_CONTROL | 0x20000000);
    #else
	ddr->sdram_cfg = CFG_DDR_CONTROL;
    #endif
	asm("sync; isync; msync");
	udelay(500);
  #endif
	return CFG_SDRAM_SIZE * 1024 * 1024;
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

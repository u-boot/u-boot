/*
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
#include <spd_sdram.h>

long int fixed_sdram (void);

int board_pre_init (void)
{
#if defined(CONFIG_PCI)
	volatile ccsr_pcix_t *pci = (void *)(CFG_MPC85xx_PCIX_ADDR);

	pci->peer &= 0xffffffdf; /* disable master abort */
#endif
	return 0;
}

int checkboard (void)
{
	sys_info_t sysinfo;

	get_sys_info (&sysinfo);

	printf ("Board: Freescale MPC8540EVAL Board\n");
	printf ("\tCPU: %lu MHz\n", sysinfo.freqProcessor / 1000000);
	printf ("\tCCB: %lu MHz\n", sysinfo.freqSystemBus / 1000000);
	printf ("\tDDR: %lu MHz\n", sysinfo.freqSystemBus / 2000000);
	if((CFG_LBC_LCRR & 0x0f) == 2 || (CFG_LBC_LCRR & 0x0f) == 4 \
		|| (CFG_LBC_LCRR & 0x0f) == 8) {
		printf ("\tLBC: %lu MHz\n",
			sysinfo.freqSystemBus / 1000000/(CFG_LBC_LCRR & 0x0f));
	} else {
		printf("\tLBC: unknown\n");
	}
	printf("L1 D-cache 32KB, L1 I-cache 32KB enabled.\n");
	return (0);
}

long int initdram (int board_type)
{
	long dram_size = 0;

#if !defined(CONFIG_RAM_AS_FLASH)
	volatile ccsr_lbc_t *lbc = (void *)(CFG_MPC85xx_LBC_ADDR);
	sys_info_t sysinfo;
	uint temp_lbcdll = 0;
#endif
#if !defined(CONFIG_RAM_AS_FLASH) || defined(CONFIG_DDR_DLL)
	volatile ccsr_gur_t *gur = (void *)(CFG_MPC85xx_GUTS_ADDR);
#endif

#if defined(CONFIG_DDR_DLL)
	uint temp_ddrdll = 0;

	/* Work around to stabilize DDR DLL */
	temp_ddrdll = gur->ddrdllcr;
	gur->ddrdllcr = ((temp_ddrdll & 0xff) << 16) | 0x80000000;
	asm("sync;isync;msync");
#endif

#if defined(CONFIG_SPD_EEPROM)
	dram_size = spd_sdram ();
#else
	dram_size = fixed_sdram ();
#endif

#if defined(CFG_RAMBOOT)
	return dram_size;
#endif

#if !defined(CONFIG_RAM_AS_FLASH) /* LocalBus is not emulating flash */
	get_sys_info(&sysinfo);
	/* if localbus freq is less than 66Mhz,we use bypass mode,otherwise use DLL */
	if(sysinfo.freqSystemBus/(CFG_LBC_LCRR & 0x0f) < 66000000) {
		lbc->lcrr = (CFG_LBC_LCRR & 0x0fffffff)| 0x80000000;
	} else {
		lbc->lcrr = CFG_LBC_LCRR & 0x7fffffff;
		udelay(200);
		temp_lbcdll = gur->lbcdllcr;
		gur->lbcdllcr = ((temp_lbcdll & 0xff) << 16 ) | 0x80000000;
		asm("sync;isync;msync");
	}
	lbc->or2 = CFG_OR2_PRELIM; /* 64MB SDRAM */
	lbc->br2 = CFG_BR2_PRELIM;
	lbc->lbcr = CFG_LBC_LBCR;
	lbc->lsdmr = CFG_LBC_LSDMR_1;
	asm("sync");
	* (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_2;
	asm("sync");
	* (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_3;
	asm("sync");
	* (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_4;
	asm("sync");
	* (ulong *)0 = 0x000000ff;
	lbc->lsdmr = CFG_LBC_LSDMR_5;
	asm("sync");
	lbc->lsrt = CFG_LBC_LSRT;
	asm("sync");
	lbc->mrtpr = CFG_LBC_MRTPR;
	asm("sync");
#endif

#if defined(CONFIG_DDR_ECC)
	{
		/* Initialize all of memory for ECC, then
		 * enable errors */
		uint *p = 0;
		uint i = 0;
		volatile ccsr_ddr_t *ddr= (void *)(CFG_MPC85xx_DDR_ADDR);
		dma_init();
		for (*p = 0; p < (uint *)(8 * 1024); p++) {
			if (((unsigned int)p & 0x1f) == 0) { dcbz(p); }
			*p = (unsigned int)0xdeadbeef;
			if (((unsigned int)p & 0x1c) == 0x1c) { dcbf(p); }
		}

		/* 8K */
		dma_xfer((uint *)0x2000,0x2000,(uint *)0);
		/* 16K */
		dma_xfer((uint *)0x4000,0x4000,(uint *)0);
		/* 32K */
		dma_xfer((uint *)0x8000,0x8000,(uint *)0);
		/* 64K */
		dma_xfer((uint *)0x10000,0x10000,(uint *)0);
		/* 128k */
		dma_xfer((uint *)0x20000,0x20000,(uint *)0);
		/* 256k */
		dma_xfer((uint *)0x40000,0x40000,(uint *)0);
		/* 512k */
		dma_xfer((uint *)0x80000,0x80000,(uint *)0);
		/* 1M */
		dma_xfer((uint *)0x100000,0x100000,(uint *)0);
		/* 2M */
		dma_xfer((uint *)0x200000,0x200000,(uint *)0);
		/* 4M */
		dma_xfer((uint *)0x400000,0x400000,(uint *)0);

		for (i = 1; i < dram_size / 0x800000; i++) {
			dma_xfer((uint *)(0x800000*i),0x800000,(uint *)0);
		}

		/* Enable errors for ECC */
		ddr->err_disable = 0x00000000;
		asm("sync;isync;msync");
	}
#endif

	return dram_size;
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
	volatile ccsr_ddr_t *ddr= (void *)(CFG_MPC85xx_DDR_ADDR);

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
	return (CFG_SDRAM_SIZE * 1024 * 1024);
}
#endif	/* !defined(CONFIG_SPD_EEPROM) */

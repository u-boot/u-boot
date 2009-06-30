/*
 * Copyright 2008 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/fsl_ddr_sdram.h>

#if (CONFIG_CHIP_SELECTS_PER_CTRL > 4)
#error Invalid setting for CONFIG_CHIP_SELECTS_PER_CTRL
#endif

void fsl_ddr_set_memctl_regs(const fsl_ddr_cfg_regs_t *regs,
			     unsigned int ctrl_num)
{
	unsigned int i;
	volatile ccsr_ddr_t *ddr = (void *)CONFIG_SYS_MPC85xx_DDR_ADDR;

	if (ctrl_num != 0) {
		printf("%s unexpected ctrl_num = %u\n", __FUNCTION__, ctrl_num);
		return;
	}

	for (i = 0; i < CONFIG_CHIP_SELECTS_PER_CTRL; i++) {
		if (i == 0) {
			out_be32(&ddr->cs0_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs0_config, regs->cs[i].config);

		} else if (i == 1) {
			out_be32(&ddr->cs1_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs1_config, regs->cs[i].config);

		} else if (i == 2) {
			out_be32(&ddr->cs2_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs2_config, regs->cs[i].config);

		} else if (i == 3) {
			out_be32(&ddr->cs3_bnds, regs->cs[i].bnds);
			out_be32(&ddr->cs3_config, regs->cs[i].config);
		}
	}

	out_be32(&ddr->timing_cfg_1, regs->timing_cfg_1);
	out_be32(&ddr->timing_cfg_2, regs->timing_cfg_2);
	out_be32(&ddr->sdram_mode, regs->ddr_sdram_mode);
	out_be32(&ddr->sdram_interval, regs->ddr_sdram_interval);
#if defined(CONFIG_MPC8555) || defined(CONFIG_MPC8541)
	out_be32(&ddr->sdram_clk_cntl, regs->ddr_sdram_clk_cntl);
#endif

	/*
	 * 200 painful micro-seconds must elapse between
	 * the DDR clock setup and the DDR config enable.
	 */
	udelay(200);
	asm volatile("sync;isync");

	out_be32(&ddr->sdram_cfg, regs->ddr_sdram_cfg);

	asm("sync;isync;msync");
	udelay(500);
}

#if defined(CONFIG_DDR_ECC) && !defined(CONFIG_ECC_INIT_VIA_DDRCONTROLLER)
/*
 * Initialize all of memory for ECC, then enable errors.
 */

void
ddr_enable_ecc(unsigned int dram_size)
{
	uint *p = 0;
	uint i = 0;
	volatile ccsr_ddr_t *ddr= (void *)(CONFIG_SYS_MPC85xx_DDR_ADDR);

	for (*p = 0; p < (uint *)(8 * 1024); p++) {
		if (((unsigned int)p & 0x1f) == 0) {
			ppcDcbz((unsigned long) p);
		}
		*p = (unsigned int)CONFIG_MEM_INIT_VALUE;
		if (((unsigned int)p & 0x1c) == 0x1c) {
			ppcDcbf((unsigned long) p);
		}
	}

	dmacpy(0x002000, 0, 0x2000); /* 8K */
	dmacpy(0x004000, 0, 0x4000); /* 16K */
	dmacpy(0x008000, 0, 0x8000); /* 32K */
	dmacpy(0x010000, 0, 0x10000); /* 64K */
	dmacpy(0x020000, 0, 0x20000); /* 128K */
	dmacpy(0x040000, 0, 0x40000); /* 256K */
	dmacpy(0x080000, 0, 0x80000); /* 512K */
	dmacpy(0x100000, 0, 0x100000); /* 1M */
	dmacpy(0x200000, 0, 0x200000); /* 2M */
	dmacpy(0x400000, 0, 0x400000); /* 4M */

	for (i = 1; i < dram_size / 0x800000; i++)
		dmacpy(0x800000 *i, 0, 0x800000);

	/*
	 * Enable errors for ECC.
	 */
	debug("DMA DDR: err_disable = 0x%08x\n", ddr->err_disable);
	ddr->err_disable = 0x00000000;
	asm("sync;isync;msync");
	debug("DMA DDR: err_disable = 0x%08x\n", ddr->err_disable);
}

#endif	/* CONFIG_DDR_ECC  && ! CONFIG_ECC_INIT_VIA_DDRCONTROLLER */

/*
 * Copyright (C) 2012 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2012 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/clk.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

#define ZYNQ_SILICON_VER_MASK	0xF0000000
#define ZYNQ_SILICON_VER_SHIFT	28

int arch_cpu_init(void)
{
	zynq_slcr_unlock();
#ifndef CONFIG_SPL_BUILD
	/* Device config APB, unlock the PCAP */
	writel(0x757BDF0D, &devcfg_base->unlock);
	writel(0xFFFFFFFF, &devcfg_base->rom_shadow);

#if (CONFIG_SYS_SDRAM_BASE == 0)
	/* remap DDR to zero, FILTERSTART */
	writel(0, &scu_base->filter_start);

	/* OCM_CFG, Mask out the ROM, map ram into upper addresses */
	writel(0x1F, &slcr_base->ocm_cfg);
	/* FPGA_RST_CTRL, clear resets on AXI fabric ports */
	writel(0x0, &slcr_base->fpga_rst_ctrl);
	/* Set urgent bits with register */
	writel(0x0, &slcr_base->ddr_urgent_sel);
	/* Urgent write, ports S2/S3 */
	writel(0xC, &slcr_base->ddr_urgent);
#endif
#endif
	zynq_clk_early_init();
	zynq_slcr_lock();

	return 0;
}

unsigned int zynq_get_silicon_version(void)
{
	unsigned int ver;

	ver = (readl(&devcfg_base->mctrl) &
	       ZYNQ_SILICON_VER_MASK) >> ZYNQ_SILICON_VER_SHIFT;

	return ver;
}

void reset_cpu(ulong addr)
{
	zynq_slcr_cpu_reset();
	while (1)
		;
}

#ifndef CONFIG_SYS_DCACHE_OFF
void enable_caches(void)
{
	/* Enable D-cache. I-cache is already enabled in start.S */
	dcache_enable();
}
#endif

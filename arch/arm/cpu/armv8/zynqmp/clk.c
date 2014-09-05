/*
 * (C) Copyright 2014 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_uart_clk(int dev_id)
{
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		return 400000;
	case ZYNQMP_CSU_VERSION_EP108:
		return 25000000;
	}

	return 133000000;
}

unsigned long get_ttc_clk(int dev_id)
{
	return get_uart_clk(dev_id);
}

#ifdef CONFIG_CLOCKS
/**
 * set_cpu_clk_info() - Initialize clock framework
 * Always returns zero.
 *
 * This function is called from common code after relocation and sets up the
 * clock framework. The framework must not be used before this function had been
 * called.
 */
int set_cpu_clk_info(void)
{
	gd->cpu_clk = get_tbclk();

	gd->bd->bi_arm_freq = gd->cpu_clk / 1000000;
	gd->bd->bi_dsp_freq = 0;

	return 0;
}
#endif

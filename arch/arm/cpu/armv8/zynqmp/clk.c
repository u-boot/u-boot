/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_uart_clk(int dev_id)
{
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		return 48000;
	case ZYNQMP_CSU_VERSION_EP108:
		return 25000000;
	case ZYNQMP_CSU_VERSION_QEMU:
		return 133000000;
	}

	return 100000000;
}

unsigned long zynqmp_get_system_timer_freq(void)
{
	u32 ver = zynqmp_get_silicon_version();

	switch (ver) {
	case ZYNQMP_CSU_VERSION_VELOCE:
		return 10000;
	case ZYNQMP_CSU_VERSION_EP108:
		return 4000000;
	case ZYNQMP_CSU_VERSION_QEMU:
		return 50000000;
	}

	return 100000000;
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

	/* Support Veloce to show at least 1MHz via bdi */
	if (gd->cpu_clk > 1000000)
		gd->bd->bi_arm_freq = gd->cpu_clk / 1000000;
	else
		gd->bd->bi_arm_freq = 1;

	gd->bd->bi_dsp_freq = 0;

	return 0;
}
#endif

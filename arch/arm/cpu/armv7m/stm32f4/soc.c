/*
 * (C) Copyright 2015
 * Kamil Lulko, <rev13@wp.pl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>

u32 get_cpu_rev(void)
{
	return 0;
}

int arch_cpu_init(void)
{
	configure_clocks();

	/*
	 * Configure the memory protection unit (MPU) to allow full access to
	 * the whole 4GB address space.
	 */
	writel(0, &V7M_MPU->rnr);
	writel(0, &V7M_MPU->rbar);
	writel((V7M_MPU_RASR_AP_RW_RW | V7M_MPU_RASR_SIZE_4GB
		| V7M_MPU_RASR_EN), &V7M_MPU->rasr);
	writel(V7M_MPU_CTRL_ENABLE | V7M_MPU_CTRL_HFNMIENA, &V7M_MPU->ctrl);

	return 0;
}

void s_init(void)
{
}

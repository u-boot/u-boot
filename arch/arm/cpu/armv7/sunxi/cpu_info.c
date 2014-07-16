/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	puts("CPU:   Allwinner A20 (SUN7I)\n");
	return 0;
}
#endif

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
#ifdef CONFIG_SUN4I
	puts("CPU:   Allwinner A10 (SUN4I)\n");
#elif defined CONFIG_SUN5I
	u32 val = readl(SUNXI_SID_BASE + 0x08);
	switch ((val >> 12) & 0xf) {
	case 0: puts("CPU:   Allwinner A12 (SUN5I)\n"); break;
	case 3: puts("CPU:   Allwinner A13 (SUN5I)\n"); break;
	case 7: puts("CPU:   Allwinner A10s (SUN5I)\n"); break;
	default: puts("CPU:   Allwinner A1X (SUN5I)\n");
	}
#elif defined CONFIG_SUN7I
	puts("CPU:   Allwinner A20 (SUN7I)\n");
#else
#warning Please update cpu_info.c with correct CPU information
	puts("CPU:   SUNXI Family\n");
#endif
	return 0;
}
#endif

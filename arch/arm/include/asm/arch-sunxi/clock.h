/*
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CLOCK_H
#define _SUNXI_CLOCK_H

#include <linux/types.h>

#define CLK_GATE_OPEN			0x1
#define CLK_GATE_CLOSE			0x0

/* clock control module regs definition */
#if defined(CONFIG_MACH_SUN6I) || defined(CONFIG_MACH_SUN8I)
#include <asm/arch/clock_sun6i.h>
#elif defined(CONFIG_MACH_SUN9I)
#include <asm/arch/clock_sun9i.h>
#else
#include <asm/arch/clock_sun4i.h>
#endif

#ifndef __ASSEMBLY__
int clock_init(void);
int clock_twi_onoff(int port, int state);
void clock_set_de_mod_clock(u32 *clk_cfg, unsigned int hz);
void clock_init_safe(void);
void clock_init_uart(void);
#endif

#endif /* _SUNXI_CLOCK_H */

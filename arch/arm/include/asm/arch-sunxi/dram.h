/*
 * (C) Copyright 2007-2012
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Berg Xing <bergxing@allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 *
 * Sunxi platform dram register definition.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_DRAM_H
#define _SUNXI_DRAM_H

#include <asm/io.h>
#include <linux/types.h>

/* dram regs definition */
#if defined(CONFIG_MACH_SUN6I)
#include <asm/arch/dram_sun6i.h>
#elif defined(CONFIG_MACH_SUN8I_A23)
#include <asm/arch/dram_sun8i_a23.h>
#elif defined(CONFIG_MACH_SUN8I_A33)
#include <asm/arch/dram_sun8i_a33.h>
#elif defined(CONFIG_MACH_SUN8I_A83T)
#include <asm/arch/dram_sun8i_a83t.h>
#elif defined(CONFIG_SUNXI_H3_DW_DRAM)
#include <asm/arch/dram_sun8i_h3.h>
#elif defined(CONFIG_MACH_SUN9I)
#include <asm/arch/dram_sun9i.h>
#else
#include <asm/arch/dram_sun4i.h>
#endif

unsigned long sunxi_dram_init(void);
void mctl_await_completion(u32 *reg, u32 mask, u32 val);
bool mctl_mem_matches(u32 offset);

#endif /* _SUNXI_DRAM_H */

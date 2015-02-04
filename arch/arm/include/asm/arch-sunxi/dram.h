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
#elif defined(CONFIG_MACH_SUN8I)
#include <asm/arch/dram_sun8i.h>
#else
#include <asm/arch/dram_sun4i.h>
#endif

unsigned long sunxi_dram_init(void);
void mctl_await_completion(u32 *reg, u32 mask, u32 val);
bool mctl_mem_matches(u32 offset);

#endif /* _SUNXI_DRAM_H */

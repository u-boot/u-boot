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
#else
#include <asm/arch/dram_sun4i.h>
#endif

#define MCTL_MEM_FILL_MATCH_COUNT 64

unsigned long sunxi_dram_init(void);

/*
 * Wait up to 1s for value to be set in given part of reg.
 */
static inline void mctl_await_completion(u32 *reg, u32 mask, u32 val)
{
	unsigned long tmo = timer_get_us() + 1000000;

	while ((readl(reg) & mask) != val) {
		if (timer_get_us() > tmo)
			panic("Timeout initialising DRAM\n");
	}
}

/*
 * Fill beginning of DRAM with "random" data for mctl_mem_matches()
 */
static inline void mctl_mem_fill(void)
{
	int i;

	for (i = 0; i < MCTL_MEM_FILL_MATCH_COUNT; i++)
		writel(0xaa55aa55 + i, CONFIG_SYS_SDRAM_BASE + i * 4);
}

/*
 * Test if memory at offset offset matches memory at begin of DRAM
 */
static inline bool mctl_mem_matches(u32 offset)
{
	int i, matches = 0;

	for (i = 0; i < MCTL_MEM_FILL_MATCH_COUNT; i++) {
		if (readl(CONFIG_SYS_SDRAM_BASE + i * 4) ==
		    readl(CONFIG_SYS_SDRAM_BASE + offset + i * 4))
			matches++;
	}

	return matches == MCTL_MEM_FILL_MATCH_COUNT;
}

#endif /* _SUNXI_DRAM_H */

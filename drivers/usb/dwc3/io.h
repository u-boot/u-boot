/* SPDX-License-Identifier: GPL-2.0 */
/*
 * io.h - DesignWare USB3 DRD IO Header
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - https://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 */

#ifndef __DRIVERS_USB_DWC3_IO_H
#define __DRIVERS_USB_DWC3_IO_H

#include <asm/io.h>
#include <cpu_func.h>

#define CACHELINE_SIZE CONFIG_SYS_CACHELINE_SIZE

static inline u32 dwc3_readl(void __iomem *base, u32 offset)
{
	u32 value;

	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */
	value = readl(base + offset - DWC3_GLOBALS_REGS_START);

	return value;
}

static inline void dwc3_writel(void __iomem *base, u32 offset, u32 value)
{
	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */
	writel(value, base + offset - DWC3_GLOBALS_REGS_START);
}

static inline void dwc3_flush_cache(uintptr_t addr, int length)
{
	uintptr_t start_addr = (uintptr_t)addr & ~(CACHELINE_SIZE - 1);
	uintptr_t end_addr = ALIGN((uintptr_t)addr + length, CACHELINE_SIZE);

	flush_dcache_range((unsigned long)start_addr, (unsigned long)end_addr);
}

#endif /* __DRIVERS_USB_DWC3_IO_H */

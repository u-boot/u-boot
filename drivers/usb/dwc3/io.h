/* SPDX-License-Identifier: GPL-2.0 */
/**
 * io.h - DesignWare USB3 DRD IO Header
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - https://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Taken from Linux Kernel v3.19-rc1 (drivers/usb/dwc3/io.h) and ported
 * to uboot.
 *
 * commit 2c4cbe6e5a : usb: dwc3: add tracepoints to aid debugging
 *
 */

#ifndef __DRIVERS_USB_DWC3_IO_H
#define __DRIVERS_USB_DWC3_IO_H

#include <cpu_func.h>
#include <asm/io.h>

#define	CACHELINE_SIZE		CONFIG_SYS_CACHELINE_SIZE
static inline u32 dwc3_readl(void __iomem *base, u32 offset)
{
	unsigned long offs = offset - DWC3_GLOBALS_REGS_START;
	u32 value;

	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */
	value = readl(base + offs);

	return value;
}

static inline void dwc3_writel(void __iomem *base, u32 offset, u32 value)
{
	unsigned long offs = offset - DWC3_GLOBALS_REGS_START;

	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */
	writel(value, base + offs);
}

static inline void dwc3_flush_cache(uintptr_t addr, int length)
{
	uintptr_t start_addr = (uintptr_t)addr & ~(CACHELINE_SIZE - 1);
	uintptr_t end_addr = ALIGN((uintptr_t)addr + length, CACHELINE_SIZE);

	flush_dcache_range((unsigned long)start_addr, (unsigned long)end_addr);
}

static inline void dwc3_invalidate_cache(uintptr_t addr, int length)
{
	uintptr_t start_addr = (uintptr_t)addr & ~(CACHELINE_SIZE - 1);
	uintptr_t end_addr = ALIGN((uintptr_t)addr + length, CACHELINE_SIZE);

	invalidate_dcache_range((unsigned long)start_addr, (unsigned long)end_addr);
}
#endif /* __DRIVERS_USB_DWC3_IO_H */

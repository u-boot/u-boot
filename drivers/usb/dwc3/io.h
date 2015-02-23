/**
 * io.h - DesignWare USB3 DRD IO Header
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com
 *
 * Authors: Felipe Balbi <balbi@ti.com>,
 *	    Sebastian Andrzej Siewior <bigeasy@linutronix.de>
 *
 * Taken from Linux Kernel v3.19-rc1 (drivers/usb/dwc3/io.h) and ported
 * to uboot.
 *
 * commit 2c4cbe6e5a : usb: dwc3: add tracepoints to aid debugging
 *
 * SPDX-License-Identifier:     GPL-2.0
 *
 */

#ifndef __DRIVERS_USB_DWC3_IO_H
#define __DRIVERS_USB_DWC3_IO_H

#include <linux/io.h>
#include "trace.h"
#include "debug.h"
#include "core.h"

static inline u32 dwc3_readl(void __iomem *base, u32 offset)
{
	u32 offs = offset - DWC3_GLOBALS_REGS_START;
	u32 value;

	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */
	value = readl(base + offs);

	/*
	 * When tracing we want to make it easy to find the correct address on
	 * documentation, so we revert it back to the proper addresses, the
	 * same way they are described on SNPS documentation
	 */
	dwc3_trace(trace_dwc3_readl, "addr %p value %08x",
			base - DWC3_GLOBALS_REGS_START + offset, value);

	return value;
}

static inline void dwc3_writel(void __iomem *base, u32 offset, u32 value)
{
	u32 offs = offset - DWC3_GLOBALS_REGS_START;

	/*
	 * We requested the mem region starting from the Globals address
	 * space, see dwc3_probe in core.c.
	 * However, the offsets are given starting from xHCI address space.
	 */
	writel(value, base + offs);

	/*
	 * When tracing we want to make it easy to find the correct address on
	 * documentation, so we revert it back to the proper addresses, the
	 * same way they are described on SNPS documentation
	 */
	dwc3_trace(trace_dwc3_writel, "addr %p value %08x",
			base - DWC3_GLOBALS_REGS_START + offset, value);
}

#endif /* __DRIVERS_USB_DWC3_IO_H */

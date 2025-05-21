// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Yao Zi <ziyao@disroot.org>
 *
 * TH1520 SoC has a set of undocumented customized PMP registers that are
 * configured through MMIO operation. It must be disabled before entering
 * the DRAM region, or an exception will be raised.
 */

#include <asm/io.h>
#include <cpu_func.h>

#define TH1520_PMP_BASE		(void *)0xffdc020000

void th1520_invalidate_pmp(void)
{
	/* Invalidate the PMP configuration as in vendor U-Boot code */
	writel(0x0, TH1520_PMP_BASE + 0x0);

	invalidate_icache_all();
}

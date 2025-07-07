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
#include <linux/bitops.h>

#define TH1520_C910_RST			(void __iomem *)(0xffef014000 + 0x004)
#define  TH1520_C910_CORE_RST_N(n)	BIT((n) + 1)
#define TH1520_SYSCFG_AP_BASE		(void __iomem *)(0xffef018000)
#define TH1520_SYSCFG_CORE_START_L(n)	(TH1520_SYSCFG_AP_BASE + 0x50 + 8 * (n))
#define TH1520_SYSCFG_CORE_START_H(n)	(TH1520_SYSCFG_AP_BASE + 0x54 + 8 * (n))
#define TH1520_PMP_BASE			(void *)0xffdc020000

void th1520_kick_secondary_cores(void)
{
	int i;

	/*
	 * On coldboot, only HART 0 is brought up by hardware, and resets for
	 * secondary cores are asserted. Set reset address of secondary cores
	 * to the entry of SPL, then deassert the resets to bring them up.
	 */
	for (i = 1; i < 4; i++) {
		writel(CONFIG_SPL_TEXT_BASE & 0xffffffff,
		       TH1520_SYSCFG_CORE_START_L(i));
		writel(CONFIG_SPL_TEXT_BASE >> 32,
		       TH1520_SYSCFG_CORE_START_H(i));
	}

	setbits_le32(TH1520_C910_RST, TH1520_C910_CORE_RST_N(1) |
				      TH1520_C910_CORE_RST_N(2) |
				      TH1520_C910_CORE_RST_N(3));
}

void th1520_invalidate_pmp(void)
{
	/* Invalidate the PMP configuration as in vendor U-Boot code */
	writel(0x0, TH1520_PMP_BASE + 0x0);

	invalidate_icache_all();
}

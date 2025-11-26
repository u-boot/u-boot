// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Portions Copyright (C) 2015, Amlogic, Inc. All rights reserved.
 * Copyright (C) 2023-2025, Ferass El Hafidi <funderscore@postmarketos.org>
 */
#include <init.h>
#include <asm/unaligned.h>
#include <linux/libfdt.h>
#include <config.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/dram-gx.h>
#include <asm/arch/gx.h>
#include <asm/arch/clock-gx.h>
#include <asm/arch/dram-settings-gx.h>
#include <linux/delay.h>

/* Meson GXBB specific DRAM init */

void meson_dram_prepare_pctl(void)
{
	writel(PCTL0_1US_PCK, PCTL_TOGCNT1U);
	writel(PCTL0_100NS_PCK, PCTL_TOGCNT100N);
	writel(PCTL0_INIT_US, PCTL_TINIT);
	writel(PCTL0_RSTH_US, PCTL_TRSTH);

	writel(PCTL0_MCFG | (CONFIG_DRAM_2T_MODE ? 8 : 0),
	       PCTL_MCFG);
	writel(PCTL0_MCFG1, PCTL_MCFG1);
	udelay(500);

	WAIT_FOR(PCTL_DFISTSTAT0);

	/* Ask the DRAM to kindly power on and wait until it is ready */
	writel(PCTL_POWCTL_POWERON, PCTL_POWCTL);
	WAIT_FOR(PCTL_POWSTAT);
}

void meson_dram_phy_init(void)
{
	/* Some unknown magic done by bl2 */
	writel(0x49494949, DDR0_PUB_IOVCR0);
	writel(0x49494949, DDR0_PUB_IOVCR1);

	writel(PUB_ODTCR, DDR0_PUB_ODTCR);

	writel(PUB_MR0, DDR0_PUB_MR0);
	writel(PUB_MR1, DDR0_PUB_MR1);
	writel(PUB_MR2, DDR0_PUB_MR2);
	writel(PUB_MR3, DDR0_PUB_MR3);

	/* Configure DRAM timing parameters (DTPR) */
	writel(PUB_DTPR0, DDR0_PUB_DTPR0);
	writel(PUB_DTPR1, DDR0_PUB_DTPR1);
	writel(PUB_PGCR1, DDR0_PUB_PGCR1);
	writel(PUB_DTPR2, DDR0_PUB_DTPR2);
	writel(PUB_DTPR3, DDR0_PUB_DTPR3);

	if (IS_ENABLED(CONFIG_DRAM_TWO_IDENTICAL_RANKS))
		writel(PUB_PGCR2 | (1 << 28), DDR0_PUB_PGCR2);
	else
		writel(PUB_PGCR2, DDR0_PUB_PGCR2);

	writel(PUB_PGCR3, DDR0_PUB_PGCR3);
	writel(PUB_DXCCR, DDR0_PUB_DXCCR);

	writel(PUB_DTCR, DDR0_PUB_DTCR);
	/* Wait for DLL lock */
	WAIT_FOR(DDR0_PUB_PGSR0);

	writel(0, DDR0_PUB_ACIOCR1);
	writel(0, DDR0_PUB_ACIOCR2);
	writel(0, DDR0_PUB_ACIOCR3);
	writel(0, DDR0_PUB_ACIOCR4);
	writel(0, DDR0_PUB_ACIOCR5);

	writel(0, DDR0_PUB_DX0GCR1);
	writel(0, DDR0_PUB_DX0GCR2);
	writel((1 << 10) | (2 << 12), DDR0_PUB_DX0GCR3);
	writel(0, DDR0_PUB_DX1GCR1);
	writel(0, DDR0_PUB_DX1GCR2);
	writel((1 << 10) | (2 << 12), DDR0_PUB_DX1GCR3);
	writel(0, DDR0_PUB_DX2GCR1);
	writel(0, DDR0_PUB_DX2GCR2);
	writel((1 << 10) | (2 << 12), DDR0_PUB_DX2GCR3);
	writel(0, DDR0_PUB_DX3GCR1);
	writel(0, DDR0_PUB_DX3GCR2);
	writel((1 << 10) | (2 << 12), DDR0_PUB_DX3GCR3);

	writel(PUB_DCR, DDR0_PUB_DCR);

	writel(PUB_DTAR, DDR0_PUB_DTAR0);
	writel(PUB_DTAR | 0x8, DDR0_PUB_DTAR1);
	writel(PUB_DTAR | 0x10, DDR0_PUB_DTAR2);
	writel(PUB_DTAR | 0x18, DDR0_PUB_DTAR3);

	writel(PUB_DSGCR, DDR0_PUB_DSGCR);

	/* Wait for the SDRAM to initialise */
	WAIT_FOR(DDR0_PUB_PGSR0);
}

void meson_dram_phy_setup_ranks(void)
{
	if (IS_ENABLED(CONFIG_DRAM_ONE_RANK) || IS_ENABLED(CONFIG_DRAM_TWO_DIFF_RANKS)) {
		uint i = 0, j = 0;

		writel((readl(DDR0_PUB_DX0LCDLR0) >> 8) |
			(readl(DDR0_PUB_DX0LCDLR0) & 0xffffff00),
			DDR0_PUB_DX0LCDLR0);

		i = ((readl(DDR0_PUB_DX2GTR) >> 3) & (7 << 0));
		j = ((readl(DDR0_PUB_DX2GTR) >> 14) & (3 << 0));
		writel(i | (i << 3) | (j << 12) | (j << 14), DDR0_PUB_DX2GTR);

		writel((readl(DDR0_PUB_DX2LCDLR2) >> 8) |
			(readl(DDR0_PUB_DX2LCDLR2) & 0xffffff00),
			DDR0_PUB_DX2LCDLR2);

		writel((readl(DDR0_PUB_DX3LCDLR0) >> 8) |
			(readl(DDR0_PUB_DX3LCDLR0) & 0xffffff00),
			DDR0_PUB_DX3LCDLR0);

		i = (readl(DDR0_PUB_DX3GTR) >> 3) & (7 << 0);
		j = (readl(DDR0_PUB_DX3GTR) >> 14) & (3 << 0);
		writel(i | (i << 3) | (j << 12) | (j << 14), DDR0_PUB_DX3GTR);

		writel((readl(DDR0_PUB_DX3LCDLR2) >> 8) |
			(readl(DDR0_PUB_DX3LCDLR2) & 0xffffff00),
			DDR0_PUB_DX3LCDLR2);

		writel((readl(DDR0_PUB_DX0LCDLR0) << 8) |
			(readl(DDR0_PUB_DX0LCDLR0) & 0xffff00ff),
			DDR0_PUB_DX0LCDLR0);

		i = (readl(DDR0_PUB_DX0GTR) << 0) & (7 << 0);
		j = (readl(DDR0_PUB_DX0GTR) >> 12) & (3 << 0);
		writel(i | (i << 3) | (j << 12) | (j << 14), DDR0_PUB_DX0GTR);

		writel((readl(DDR0_PUB_DX0LCDLR2) << 8) |
			(readl(DDR0_PUB_DX0LCDLR2) & 0xffff00ff),
			DDR0_PUB_DX0LCDLR2);

		writel((readl(DDR0_PUB_DX1LCDLR0) << 8) |
			(readl(DDR0_PUB_DX1LCDLR0) & 0xffff00ff),
			DDR0_PUB_DX1LCDLR0);

		i = (readl(DDR0_PUB_DX1GTR) << 0) & (7 << 0);
		j = (readl(DDR0_PUB_DX1GTR) >> 12) & (3 << 0);
		writel(i | (i << 3) | (j << 12) | (j << 14), DDR0_PUB_DX0GTR);

		writel((readl(DDR0_PUB_DX1LCDLR2) >> 8) |
			(readl(DDR0_PUB_DX1LCDLR2) & 0xffffff00),
			DDR0_PUB_DX1LCDLR2);
	}

	writel((~(1 << 28)) & PUB_PGCR2, DDR0_PUB_PGCR2);

	if (IS_ENABLED(CONFIG_DRAM_2T_MODE) && (PUB_DCR & 7) == 3)
		writel(0x1f, DDR0_PUB_ACLCDLR);
}

void meson_dram_finalise_init(void)
{
	WAIT_FOR(PCTL_CMDTSTAT);
	writel(PCTL_SCTL_GO_STATE, PCTL_SCTL);

	while (readl(PCTL_STAT) != PCTL_STAT_ACCESS)
		;
	writel(0x880019d, DMC_REFR_CTRL1);
	writel(0x20100000 | (CONFIG_DRAM_CLK / 20) |
		(timings.refi << 8), DMC_REFR_CTRL2);
	clrbits_32(DDR0_PUB_ZQCR, 4);
}

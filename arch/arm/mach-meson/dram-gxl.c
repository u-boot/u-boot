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

/* Meson GXL specific DRAM init */

void meson_dram_prepare_pctl(void)
{
	writel(CONFIG_DRAM_2T_MODE ? 8 : 0, PCTL_MCFG);
	setbits_32(PCTL_MCFG, PCTL0_MCFG);
	udelay(500);

	WAIT_FOR(PCTL_DFISTSTAT0);

	/* Enter config state */
	writel(PCTL0_SCFG, PCTL_SCFG);
	writel(PCTL_SCTL_CFG_STATE, PCTL_SCTL);
	WAIT_FOR(PCTL_STAT);

	writel(0x581, DDR0_PUB_PIR);
	WAIT_FOR(DDR0_PUB_PGSR0);
}

void meson_dram_phy_init(void)
{
	/* Some unknown magic done by bl2 */
	writel(0x190c3500, DDR0_PUB_PTR3);
	writel(0x12c493e0, DDR0_PUB_PTR4);

	writel(0x1f090909, DDR0_PUB_IOVCR0);
	writel(0x109, DDR0_PUB_IOVCR1);

	writel(0xe09093c, DDR0_PUB_DX0GCR4);
	writel(0xe09093c, DDR0_PUB_DX1GCR4);
	writel(0xe09093c, DDR0_PUB_DX2GCR4);
	writel(0xe09093c, DDR0_PUB_DX3GCR4);

	writel(PUB_ODTCR, DDR0_PUB_ODTCR);

	writel(PUB_MR0, DDR0_PUB_MR0);
	writel(PUB_MR1, DDR0_PUB_MR1);
	writel(PUB_MR2, DDR0_PUB_MR2);
	writel(PUB_MR3, DDR0_PUB_MR3);
	writel(PUB_MR4, DDR0_PUB_MR4);
	writel(PUB_MR5, DDR0_PUB_MR5);
	writel(PUB_MR6, DDR0_PUB_MR6);

	/* Configure DRAM timing parameters (DTPR) */
	writel(timings.odt | (1 << 2), DDR0_PUB_MR11);
	writel(PUB_DTPR0, DDR0_PUB_DTPR0);
	writel(PUB_DTPR1, DDR0_PUB_DTPR1);
	writel(PUB_DTPR2, DDR0_PUB_DTPR2);
	writel(PUB_DTPR3, DDR0_PUB_DTPR3);
	writel(PUB_DTPR4, DDR0_PUB_DTPR4);
	writel(PUB_DTPR5, DDR0_PUB_DTPR5);

	if (IS_ENABLED(CONFIG_DRAM_TWO_IDENTICAL_RANKS))
		writel(PUB_PGCR2 | (1 << 28), DDR0_PUB_PGCR2);
	else
		writel(PUB_PGCR2, DDR0_PUB_PGCR2);

	writel(PUB_PGCR3, DDR0_PUB_PGCR3);
	writel(PUB_DXCCR, DDR0_PUB_DXCCR);

	writel(PUB_DTCR, DDR0_PUB_DTCR);
	writel(PUB_DTCR1, DDR0_PUB_DTCR1);
	writel(PUB_PGCR1, DDR0_PUB_PGCR1);

	writel(0, DDR0_PUB_ACIOCR1);
	writel(0, DDR0_PUB_ACIOCR2);
	writel(0, DDR0_PUB_ACIOCR3);
	writel(0, DDR0_PUB_ACIOCR4);
	writel(0, DDR0_PUB_ACIOCR5);

	writel(0, DDR0_PUB_DX0GCR1);
	writel(0, DDR0_PUB_DX0GCR2);
	writel(0, DDR0_PUB_DX1GCR1);
	writel(0, DDR0_PUB_DX1GCR2);
	writel(0, DDR0_PUB_DX2GCR1);
	writel(0, DDR0_PUB_DX2GCR2);
	writel(0, DDR0_PUB_DX3GCR1);
	writel(0, DDR0_PUB_DX3GCR2);

	if (IS_ENABLED(CONFIG_DRAM_16BIT_RANK)) {
		writel(0, DDR0_PUB_DX2GCR0);
		writel(0, DDR0_PUB_DX3GCR0);
	}
	writel(0x73, DDR0_PUB_PIR);
	WAIT_FOR(DDR0_PUB_PGSR0);

	writel(PUB_DCR | (CONFIG_DRAM_2T_MODE ? 1 << 28 : 0), DDR0_PUB_DCR);

	writel(0xfc00172, DDR0_PUB_VTCR1);

	writel(PUB_DSGCR & ~(0x800004), DDR0_PUB_DSGCR);

	/* Wait for the SDRAM to initialise */
	WAIT_FOR(DDR0_PUB_PGSR0);
}

void meson_dram_phy_setup_ranks(void)
{
	if (IS_ENABLED(CONFIG_DRAM_2T_MODE)) {
		writel(0x3f003f, DDR0_PUB_ACBDLR2);
		if (PUB_ACLCDLR <= 62) {
			writel(((PUB_ACLCDLR - 24) + (readl(DDR0_PUB_ACMDLR0) & ~(0xe00))) |
				(((PUB_ACLCDLR - 24) + (readl(DDR0_PUB_ACMDLR0) & ~(0xe00)))
				* 0xffff), DDR0_PUB_ACBDLR2);
		}
	}
	writel((PUB_ACLCDLR - 24) + (readl(DDR0_PUB_ACMDLR0) & ~(0xe00)), DDR0_PUB_ACLCDLR);

	if (IS_ENABLED(CONFIG_DRAM_DQS_CORR)) {
		/* DQS correction stuff(?) */
		clrbits_32(DDR0_PUB_ACLCDLR, 0xe00);
		if (!readl(DDR0_PUB_ACLCDLR))
			writel(1, DDR0_PUB_ACLCDLR);

		writel(readl(DDR0_PUB_ACLCDLR) & ~(0xe00), DDR0_PUB_ACLCDLR);

		clrbits_32(DDR0_PUB_ACBDLR0, 0xe00);
		if (!readl(DDR0_PUB_ACBDLR0))
			writel(1, DDR0_PUB_ACBDLR0);

		writel(readl(DDR0_PUB_ACBDLR0) & ~(0xc0), DDR0_PUB_ACBDLR0);

		DQSCORR_DX(DDR0_PUB_DX0LCDLR0);
		DQSCORR_DX(DDR0_PUB_DX1LCDLR0);
		DQSCORR_DX(DDR0_PUB_DX2LCDLR0);
		DQSCORR_DX(DDR0_PUB_DX3LCDLR0);
	}
}

void meson_dram_finalise_init(void)
{
	writel((0x3f << 12) | 0xf8, DDR0_PUB_PGCR6);
	writel(PCTL_SCTL_GO_STATE, PCTL_SCTL);
	while ((readl(PCTL_STAT) & 7) != PCTL_STAT_ACCESS)
		;

	writel(0xfffc << 16, DDR0_PUB_DX0GCR3);
	writel(0xfffc << 16, DDR0_PUB_DX1GCR3);
	writel(0xfffc << 16, DDR0_PUB_DX2GCR3);
	writel(0xfffc << 16, DDR0_PUB_DX3GCR3);

	writel(0, DDR0_PUB_RANKIDR);
	writel(PUB_DSGCR | 0x800004, DDR0_PUB_DSGCR);

	setbits_32(DDR0_PUB_ZQCR, 4);
	writel(0x20100000 | ((CONFIG_DRAM_CLK / 20) - 1) |
		(timings.refi << 8), DMC_REFR_CTRL2);
	writel(0xf08f, DMC_REFR_CTRL1);
}


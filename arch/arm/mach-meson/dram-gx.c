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
#include <asm/arch/dram-timings-gx.h>
#include <linux/delay.h>

/*
 * Meson GX common shared DRAM init code
 *
 * See dram-gxbb.c and dram-gxl.c for gxbb/gxl-specific code
 */

void meson_dram_pll_init(void)
{
	setbits_32(AM_ANALOG_TOP_REG1, 1);
	setbits_32(GX_HIU_BASE + HHI_MPLL_CNTL5, 1);

	clrbits_32(AM_DDR_PLL_CNTL4, BIT(12));
	setbits_32(AM_DDR_PLL_CNTL4, BIT(12));

	udelay(10);

	do {
		if (IS_ENABLED(CONFIG_MESON_GXBB))
			writel(1 << 29, AM_DDR_PLL_CNTL0);
		writel(DDR_PLL_CNTL1, AM_DDR_PLL_CNTL1);
		writel(DDR_PLL_CNTL2, AM_DDR_PLL_CNTL2);
		writel(DDR_PLL_CNTL3, AM_DDR_PLL_CNTL3);
		writel(DDR_PLL_CNTL4, AM_DDR_PLL_CNTL4);
		if (IS_ENABLED(CONFIG_MESON_GXBB)) {
			if (CONFIG_DRAM_CLK >= 375 && CONFIG_DRAM_CLK <= 749)
				writel((1 << 29) | ((2 << 16) | (1 << 9) |
					(((CONFIG_DRAM_CLK / 6) * 6) / 12)), AM_DDR_PLL_CNTL0);
			else if (CONFIG_DRAM_CLK >= 750 && CONFIG_DRAM_CLK <= 1449)
				writel((1 << 29) | ((1 << 16) | (1 << 9) |
					(((CONFIG_DRAM_CLK / 12) * 12) / 24)), AM_DDR_PLL_CNTL0);
			clrbits_32(AM_DDR_PLL_CNTL0, 1 << 29);
		} else if (IS_ENABLED(CONFIG_MESON_GXL)) {
			writel(DDR_PLL_CNTL5, AM_DDR_PLL_CNTL5);
			if (CONFIG_DRAM_CLK >= 399 && CONFIG_DRAM_CLK <= 799)
				writel(((1 << 16) | ((1 << 2) | 1) |
					((CONFIG_DRAM_CLK / 12) << 4)) |
					((1 << 31) | (1 << 29) | (1 << 28)), AM_DDR_PLL_CNTL0);
			else if (CONFIG_DRAM_CLK >= 800 && CONFIG_DRAM_CLK <= 1498)
				writel(((1 << 16) | (1 << 2) |
					((CONFIG_DRAM_CLK / 24) << 4)) |
					((1 << 31) | (1 << 29) | (1 << 28)), AM_DDR_PLL_CNTL0);
		}
		udelay(200);
	} while (!((readl(AM_DDR_PLL_STS) >> 0x1F) & 1));

	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		writel(DDR_CLK_CNTL_CLKGEN_SOFTRESET |
			DDR_CLK_CNTL_PHY_CLK_ENABLE |
			DDR_CLK_CNTL_DDRPLL_ENABLE, DDR_CLK_CNTL);
	} else if (IS_ENABLED(CONFIG_MESON_GXL)) {
		writel(DDR_CLK_CNTL_CLKGEN_SOFTRESET |
			DDR_CLK_CNTL_PHY_CLK_ENABLE |
			DDR_CLK_CNTL_DDRPLL_ENABLE |
			0xa005 /* unknown */, DDR_CLK_CNTL);
	}

	printf("DRAM clock: %d MHz\n", CONFIG_DRAM_CLK);
}

void meson_dram_phy_prepare(void)
{
	/* Release reset of DLL */
	writel(0xffffffff, DMC_SOFT_RST);
	writel(0xffffffff, DMC_SOFT_RST1);
	udelay(10);

	/* Enable UPCTL and PUB clock */
	if (IS_ENABLED(CONFIG_MESON_GXBB))
		writel(0x550620, DMC_PCTL_LP_CTRL);
	else if (IS_ENABLED(CONFIG_MESON_GXL))
		writel(0, DMC_PCTL_LP_CTRL);
	writel(0xf, DDR0_SOFT_RESET);
	udelay(10);
}

void meson_dram_set_memory_timings(void)
{
	/* Set memory timings */
	writel(timings.rfc, PCTL_TRFC);
	if (IS_ENABLED(CONFIG_MESON_GXL))
		writel(timings.faw, PCTL_TFAW);
	writel(timings.refi_mddr3, PCTL_TREFI_MEM_DDR3);
	writel(timings.mrd, PCTL_TMRD);
	if (IS_ENABLED(CONFIG_MESON_GXL))
		writel((timings.rp << 16) | timings.rp, PCTL_TRP);
	else /* Meson GXBB */
		writel(timings.rp, PCTL_TRP);
	writel(timings.cke + 1, PCTL_TCKESR);
	writel(timings.al, PCTL_TAL);
	writel(timings.cwl, PCTL_TCWL);
	writel(timings.cl, PCTL_TCL);
	writel(timings.ras, PCTL_TRAS);
	writel(timings.rc, PCTL_TRC);
	writel(timings.rcd, PCTL_TRCD);
	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		writel(timings.rrd, PCTL_TRRD);
	} else {
		writel(timings.rrd | ((timings.rrd + 2) * 0x10000), PCTL_TRRD);
		writel((timings.tccdl << 16) | 4, PCTL_TCCD);
	}
	writel(timings.rtp, PCTL_TRTP);
	writel(timings.wr, PCTL_TWR);
	writel(timings.wtr, PCTL_TWTR);
	writel(timings.exsr, PCTL_TEXSR);
	writel(timings.xp, PCTL_TXP);
	writel(timings.dqs, PCTL_TDQS);
	writel(timings.rtw, PCTL_TRTW);
	writel(timings.cksre, PCTL_TCKSRE);
	writel(timings.cksrx, PCTL_TCKSRX);
	writel(timings.mod, PCTL_TMOD);
	writel(timings.cke, PCTL_TCKE);
	writel(timings.zqcs, PCTL_TZQCS);
	writel(timings.zqcl, PCTL_TZQCL);
	writel(timings.xpdll, PCTL_TXPDLL);
	writel(timings.zqcsi, PCTL_TZQCSI);

	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		/* GXBB: Enter config state */
		writel(PCTL0_SCFG, PCTL_SCFG);
		writel(PCTL_SCTL_CFG_STATE, PCTL_SCTL);
	}
}

void meson_dram_set_dfi_timings(void)
{
#ifdef CONFIG_MESON_GXL
	writel(0xab0a560a, PCTL_ZQCFG);
#endif
	WAIT_FOR(PCTL_STAT);

	writel(PCTL0_PPCFG, PCTL_PPCFG);
	writel(PCTL0_DFISTCFG0, PCTL_DFISTCFG0);
	writel(PCTL0_DFISTCFG1, PCTL_DFISTCFG1);
	writel(PCTL0_DFITCTRLDELAY, PCTL_DFITCTRLDELAY);
	writel(PCTL0_DFITPHYWRDATA, PCTL_DFITPHYWRDATA);
	writel(PCTL0_DFITPHYWRLTA, PCTL_DFITPHYWRLAT);
	writel(PCTL0_DFITRDDATAEN, PCTL_DFITRDDATAEN);
	writel(PCTL0_DFITPHYRDLAT, PCTL_DFITPHYRDLAT);
	writel(PCTL0_DFITDRAMCLKDIS, PCTL_DFITDRAMCLKDIS);
	writel(PCTL0_DFITDRAMCLKEN, PCTL_DFITDRAMCLKEN);
	writel(PCTL0_DFITCTRLUPDMIN, PCTL_DFITCTRLUPDMIN);
#if defined(CONFIG_MESON_GXL)
	writel(PCTL0_DFITCTRLUPDMAX, PCTL_DFITCTRLUPDMAX);
	writel(PCTL0_DFIUPDCFG, PCTL_DFIUPDCFG);
#endif
	writel(PCTL0_DFILPCFG0, PCTL_DFILPCFG0);
#if defined(CONFIG_MESON_GXL)
	writel(PCTL0_DFITPHYUPDTYPE0, PCTL_DFITPHYUPDTYPE0);
#endif
	writel(PCTL0_DFITPHYUPDTYPE1, PCTL_DFITPHYUPDTYPE1);
	writel(PCTL0_DFIODTCFG, PCTL_DFIODTCFG);
	writel(PCTL0_DFIODTCFG1, PCTL_DFIODTCFG1);
#if defined(CONFIG_MESON_GXBB)
	writel(PCTL0_CMDTSTATEN, PCTL_CMDTSTATEN);
#endif
}

uint meson_dram_phy_finalise_init(void)
{
	writel(PUB_ZQ0PR, DDR0_PUB_ZQ0PR);
	writel(PUB_ZQ1PR, DDR0_PUB_ZQ1PR);
	writel(PUB_ZQ2PR, DDR0_PUB_ZQ2PR);
#if defined(CONFIG_MESON_GXBB)
	writel(PUB_ZQ3PR, DDR0_PUB_ZQ3PR);
#endif

	writel(PUB_PIR_INIT | PUB_PIR_ZCAL, DDR0_PUB_PIR);
	WAIT_FOR(DDR0_PUB_PGSR0);
	/*
	 * Is this needed?
	 * TODO: test without
	 */
	writel(readl(DDR0_PUB_ZQCR) | (1 << 2) | (1 << 27), DDR0_PUB_ZQCR);
	udelay(10);
	writel(readl(DDR0_PUB_ZQCR) & ~((1 << 2) | (1 << 27)), DDR0_PUB_ZQCR);
	udelay(30);

#if defined(CONFIG_MESON_GXL) && defined(CONFIG_DRAM_16BIT_RANK)
	clrsetbits_32(DDR0_PUB_DX2GCR0, 0xb0001, 0xb0000); /* Make it neat somehow? */
	clrsetbits_32(DDR0_PUB_DX3GCR0, 0xb0001, 0xb0000);
#endif

	writel(PUB_ACBDLR0, DDR0_PUB_ACBDLR0);
#if defined(CONFIG_MESON_GXL)
	writel(PUB_ACBDLR3, DDR0_PUB_ACBDLR3);
#endif

#if defined(CONFIG_MESON_GXL) && defined(CONFIG_DRAM_2T_MODE) && defined(CONFIG_DRAM_DDR3)
	writel(0x10101010, DDR0_PUB_ACBDLR1);
	writel(0x10101010, DDR0_PUB_ACBDLR7);
	writel(0x20202020, DDR0_PUB_ACBDLR8);
	writel(0x30303030, DDR0_PUB_ACBDLR9);
	writel(0x3f003f, DDR0_PUB_ACBDLR2);
	writel(0, DDR0_PUB_ACBDLR6);
#endif

#if defined(CONFIG_MESON_GXL) && defined(CONFIG_DRAM_DDR3)
	clrsetbits_32(DDR0_PUB_DXCCR, (3 << 5) | (3 << 7) |
		(3 << 9) | (3 << 11), (1 << 12) | (1 << 9));
#endif

	writel(PUB_PIR_FINAL_STEP, DDR0_PUB_PIR);
	udelay(1000);

	for (u32 pgsr0 = readl(DDR0_PUB_PGSR0); (pgsr0 != 0xc0000fff) &&
	     (pgsr0 != 0x80000fff); pgsr0 = readl(DDR0_PUB_PGSR0)) {
		udelay(20);
		debug("Waiting for PGSR0, currently 0x%x\n", pgsr0);

		/* Check for errors */
		if (pgsr0 & PUB_PGSR0_ZCERR)
			pr_err("%s: impedance calibration error\n", __func__);
		if (pgsr0 & PUB_PGSR0_WLERR)
			pr_err("%s: write leveling error\n", __func__);
		if (pgsr0 & PUB_PGSR0_QSGERR)
			pr_err("%s: DQS gate training error\n", __func__);
		if (pgsr0 & PUB_PGSR0_WLAERR)
			pr_err("%s: WL Adj error\n", __func__);
		if (pgsr0 & PUB_PGSR0_RDERR)
			pr_err("%s: read bit deskew error", __func__);
		if (pgsr0 & PUB_PGSR0_WDERR)
			pr_err("%s: write bit deskew error", __func__);
		if (pgsr0 & PUB_PGSR0_REERR)
			pr_err("%s: read eye training error", __func__);
		if (pgsr0 & PUB_PGSR0_WEERR)
			pr_err("%s: write eye training error", __func__);
	}
	debug("Wait done for PGSR0, currently 0x%x\n", readl(DDR0_PUB_PGSR0));

	return 0;
}

void meson_dram_dmc_set_addrmap(void)
{
	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		/* GXBB address map */
		if (IS_ENABLED(CONFIG_DRAM_TWO_IDENTICAL_RANKS) ||
		    IS_ENABLED(CONFIG_DRAM_ONE_RANK)) {
			writel(11 | 31 << 5 |  0 << 10 | 14 << 15 | 15 << 20 | 16 << 25,
			       DDR0_ADDRMAP_1);
			writel(30 | 12 << 5 | 13 << 10 | 29 << 15 |  0 << 20 |  0 << 25,
			       DDR0_ADDRMAP_4);
		} else if (IS_ENABLED(CONFIG_DRAM_TWO_DIFF_RANKS)) {
			writel(11 | 31 << 5 |  0 << 10 | 14 << 15 | 15 << 20 | 16 << 25,
			       DDR0_ADDRMAP_1);
			writel(0 | 12 << 5 | 13 << 10 | 29 << 15 |  0 << 20 | 30 << 25,
			       DDR0_ADDRMAP_4);
		}
	} else if (IS_ENABLED(CONFIG_MESON_GXL) && IS_ENABLED(CONFIG_DRAM_DDR3)) {
		/* This applies for GXL + DDR3 RAM (e.g. LePotato) */
		if (IS_ENABLED(CONFIG_DRAM_TWO_IDENTICAL_RANKS) ||
		    IS_ENABLED(CONFIG_DRAM_ONE_RANK)) {
			writel(11 | 30 << 5 |  0 << 10 | 15 << 15 | 16 << 20 | 17 << 25,
			       DDR0_ADDRMAP_1);
			writel(18 | 19 << 5 | 20 << 10 | 21 << 15 | 22 << 20 | 23 << 25,
			       DDR0_ADDRMAP_2);
			writel(24 | 25 << 5 | 26 << 10 | 27 << 15 | 28 << 20 | 29 << 25,
			       DDR0_ADDRMAP_3);
			writel(30 | 12 << 5 | 13 << 10 | 14 << 15 |  0 << 20 | 31 << 25,
			       DDR0_ADDRMAP_4);

			writel(5 |  6 << 5 |  7 << 10 |  8 << 15 |  9 << 20 | 10 << 25,
			       DDR1_ADDRMAP_0);
			writel(11 | 30 << 5 |  0 << 10 | 15 << 15 | 16 << 20 | 17 << 25,
			       DDR1_ADDRMAP_1);
			writel(18 | 19 << 5 | 20 << 10 | 21 << 15 | 22 << 20 | 23 << 25,
			       DDR1_ADDRMAP_2);
			writel(24 | 25 << 5 | 26 << 10 | 27 << 15 | 28 << 20 | 29 << 25,
			       DDR1_ADDRMAP_3);
			writel(30 | 12 << 5 | 13 << 10 | 14 << 15 |  0 << 20 | 31 << 25,
			       DDR1_ADDRMAP_4);
		}
	} else if (IS_ENABLED(CONFIG_MESON_GXL) && IS_ENABLED(CONFIG_DRAM_DDR4)) {
		/* This applies for GXL + DDR4 RAM (e.g. LaFrite) */
		if (IS_ENABLED(CONFIG_DRAM_TWO_IDENTICAL_RANKS) ||
		    IS_ENABLED(CONFIG_DRAM_ONE_RANK)) {
			writel(6 |  7 << 5 |  8 << 10 |  9 << 15 | 10 << 20 | 11 << 25,
			       DDR0_ADDRMAP_0);
			writel(12 |  0 << 5 |  0 << 10 | 15 << 15 | 16 << 20 | 17 << 25,
			       DDR0_ADDRMAP_1);
			writel(18 | 19 << 5 | 20 << 10 | 21 << 15 | 22 << 20 | 23 << 25,
			       DDR0_ADDRMAP_2);
			writel(24 | 25 << 5 | 26 << 10 | 27 << 15 | 28 << 20 | 29 << 25,
			       DDR0_ADDRMAP_3);
			writel(30 | 13 << 5 | 14 << 10 |  5 << 15 |  0 << 20 | 31 << 25,
			       DDR0_ADDRMAP_4);

			writel(6 |  7 << 5 |  8 << 10 |  9 << 15 | 10 << 20 | 11 << 25,
			       DDR1_ADDRMAP_0);
			writel(12 |  0 << 5 |  0 << 10 | 15 << 15 | 16 << 20 | 17 << 25,
			       DDR1_ADDRMAP_1);
			writel(18 | 19 << 5 | 20 << 10 | 21 << 15 | 22 << 20 | 23 << 25,
			       DDR1_ADDRMAP_2);
			writel(24 | 25 << 5 | 26 << 10 | 27 << 15 | 28 << 20 | 29 << 25,
			       DDR1_ADDRMAP_3);
			writel(30 | 13 << 5 | 14 << 10 |  5 << 15 |  0 << 20 | 31 << 25,
			       DDR1_ADDRMAP_4);
		} else if (IS_ENABLED(CONFIG_DRAM_16BIT_RANK)) {
			writel(0 | 6 << 5  |  7 << 10 |  8 << 15 |  9 << 20 | 10 << 25,
			       DDR0_ADDRMAP_0);
			writel(11 | 0 << 5  |  0 << 10 | 14 << 15 | 15 << 20 | 16 << 25,
			       DDR0_ADDRMAP_1);
			writel(17 | 18 << 5 | 19 << 10 | 20 << 15 | 21 << 20 | 22 << 25,
			       DDR0_ADDRMAP_2);
			writel(23 | 24 << 5 | 25 << 10 | 26 << 15 | 27 << 20 | 28 << 25,
			       DDR0_ADDRMAP_3);
			writel(29 | 12 << 5 | 13 << 10 |  5 << 15 |  0 << 20 | 30 << 25,
			       DDR0_ADDRMAP_4);

			writel(0 |  6 << 5 |  7 << 10 |  8 << 15 |  9 << 20 | 10 << 25,
			       DDR1_ADDRMAP_0);
			writel(11 |  0 << 5 |  0 << 10 | 14 << 15 | 15 << 20 | 16 << 25,
			       DDR1_ADDRMAP_1);
			writel(17 | 18 << 5 | 19 << 10 | 20 << 15 | 21 << 20 | 22 << 25,
			       DDR1_ADDRMAP_2);
			writel(23 | 24 << 5 | 25 << 10 | 26 << 15 | 27 << 20 | 28 << 25,
			       DDR1_ADDRMAP_3);
			writel(29 | 12 << 5 | 13 << 10 |  5 << 15 |  0 << 20 | 30 << 25,
			       DDR1_ADDRMAP_4);
		}
	}
}

void meson_dram_dmc_init(void)
{
	u32 ddr_size_register = 0;

	printf("DMC version: 0x%x\n", readl(DMC_VERSION));

	for (int i = CONFIG_DRAM_SIZE >> DMC_DRAM_SIZE_SHIFT;
	     !((i >>= 1) & 1); ddr_size_register++)
		;

	if (IS_ENABLED(CONFIG_DRAM_TWO_IDENTICAL_RANKS) || IS_ENABLED(CONFIG_DRAM_ONE_RANK))
		writel(DMC_CTRL | ddr_size_register |
			(ddr_size_register << 3),
			DMC_DDR_CTRL);
	else
		writel(DMC_CTRL | ddr_size_register |
			(5 << 3),
			DMC_DDR_CTRL);

	meson_dram_dmc_set_addrmap();

	if (IS_ENABLED(CONFIG_MESON_GXBB)) {
		writel(0x440620, DMC_PCTL_LP_CTRL);
		writel(BIT(13) | BIT(5), DDR0_APD_CTRL);
		writel(0x5, DDR0_CLK_CTRL);

		writel(0x11, DMC_AXI0_QOS_CTRL1);
	} else if (IS_ENABLED(CONFIG_MESON_GXL)) {
		writel(BIT(13), DDR0_APD_CTRL);
	}

	writel(0x0, DMC_SEC_RANGE_CTRL);
	writel(0x80000000, DMC_SEC_CTRL);
	writel(0x55555555, DMC_SEC_AXI_PORT_CTRL);
	writel(0x55555555, DMC_DEV_SEC_READ_CTRL);
	writel(0x55555555, DMC_DEV_SEC_WRITE_CTRL);
	writel(0x15, DMC_GE2D_SEC_CTRL);
	writel(0x5, DMC_PARSER_SEC_CTRL);
	DMC_ENABLE_REGION(DMC_VPU);
	DMC_ENABLE_REGION(DMC_VDEC);
	DMC_ENABLE_REGION(DMC_HCODEC);
	DMC_ENABLE_REGION(DMC_HEVC);

	writel(0xffff, DMC_REQ_CTRL);

	dmb();
	isb();

	debug("dram: memory controller init done\n");
}

int dram_init(void)
{
	uint ret;

	debug("SPL: initialising dram\n");

	meson_dram_pll_init();
	meson_dram_phy_prepare();
	meson_dram_phy_init();
	meson_dram_prepare_pctl();
	meson_dram_set_memory_timings();
	meson_dram_set_dfi_timings();
	ret = meson_dram_phy_finalise_init();
	if (ret < 0)
		return ret;
	meson_dram_phy_setup_ranks();
	meson_dram_finalise_init();
	meson_dram_dmc_init();

	/* Write size */
	clrsetbits_32(GX_SEC_AO_SEC_GP_CFG0, GX_AO_MEM_SIZE_MASK,
		      CONFIG_DRAM_SIZE << GX_AO_MEM_SIZE_SHIFT);

	debug("SPL: dram init done\n");

	return 0;
}

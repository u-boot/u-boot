// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments K3 SoC PLL clock driver
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 *	Tero Kristo <t-kristo@ti.com>
 */

#include <asm/io.h>
#include <dm.h>
#include <div64.h>
#include <errno.h>
#include <clk-uclass.h>
#include <linux/clk-provider.h>
#include "k3-clk.h"
#include <linux/rational.h>
#include <linux/delay.h>

/* 16FFT register offsets */
#define PLL_16FFT_CFG			0x08
#define PLL_KICK0			0x10
#define PLL_KICK1			0x14
#define PLL_16FFT_CTRL			0x20
#define PLL_16FFT_STAT			0x24
#define PLL_16FFT_FREQ_CTRL0		0x30
#define PLL_16FFT_FREQ_CTRL1		0x34
#define PLL_16FFT_DIV_CTRL		0x38
#define PLL_16FFT_CAL_CTRL		0x60
#define PLL_16FFT_CAL_STAT		0x64

/* CAL STAT register bits */
#define PLL_16FFT_CAL_STAT_CAL_LOCK	BIT(31)
#define PLL_16FFT_CAL_STAT_CAL_LOCK_TIMEOUT (4350U * 100U)

/* CFG register bits */
#define PLL_16FFT_CFG_PLL_TYPE_SHIFT	(0)
#define PLL_16FFT_CFG_PLL_TYPE_MASK	(0x3 << 0)
#define PLL_16FFT_CFG_PLL_TYPE_FRAC2	0
#define PLL_16FFT_CFG_PLL_TYPE_FRACF	1

/* CAL CTRL register bits */
#define PLL_16FFT_CAL_CTRL_CAL_EN               BIT(31)
#define PLL_16FFT_CAL_CTRL_FAST_CAL             BIT(20)
#define PLL_16FFT_CAL_CTRL_CAL_BYP              BIT(15)
#define PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT        16
#define PLL_16FFT_CAL_CTRL_CAL_CNT_MASK         (0x7 << 16)
#define PLL_16FFT_CAL_CTRL_CAL_IN_MASK          (0xFFFU)

/* CTRL register bits */
#define PLL_16FFT_CTRL_BYPASS_EN	BIT(31)
#define PLL_16FFT_CTRL_BYP_ON_LOCKLOSS	BIT(16)
#define PLL_16FFT_CTRL_PLL_EN		BIT(15)
#define PLL_16FFT_CTRL_INTL_BYP_EN	BIT(8)
#define PLL_16FFT_CTRL_CLK_4PH_EN	BIT(5)
#define PLL_16FFT_CTRL_CLK_POSTDIV_EN	BIT(4)
#define PLL_16FFT_CTRL_DSM_EN		BIT(1)
#define PLL_16FFT_CTRL_DAC_EN		BIT(0)

/* STAT register bits */
#define PLL_16FFT_STAT_LOCK		BIT(0)
#define PLL_16FFT_STAT_LOCK_TIMEOUT	(150U * 100U)

/* FREQ_CTRL0 bits */
#define PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK	0xfff

/* DIV CTRL register bits */
#define PLL_16FFT_DIV_CTRL_REF_DIV_MASK		0x3f

/* HSDIV register bits*/
#define PLL_16FFT_HSDIV_CTRL_CLKOUT_EN          BIT(15)

/* FREQ_CTRL1 bits */
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS	24
#define PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK	0xffffff

/* KICK register magic values */
#define PLL_KICK0_VALUE				0x68ef3490
#define PLL_KICK1_VALUE				0xd172bc5a

/**
 * struct ti_pll_clk - TI PLL clock data info structure
 * @clk: core clock structure
 * @reg: memory address of the PLL controller
 */
struct ti_pll_clk {
	struct clk	clk;
	void __iomem	*base;
};

#define to_clk_pll(_clk) container_of(_clk, struct ti_pll_clk, clk)

static int ti_pll_clk_disable(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u32 ctrl;

	ctrl = readl(pll->base + PLL_16FFT_CTRL);

	if ((ctrl & PLL_16FFT_CTRL_PLL_EN)) {
		ctrl &= ~PLL_16FFT_CTRL_PLL_EN;
		writel(ctrl, pll->base + PLL_16FFT_CTRL);

		/* wait 1us */
		udelay(1);
	}

	return 0;
}

static int ti_pll_clk_enable(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u32 ctrl;

	ctrl = readl(pll->base + PLL_16FFT_CTRL);
	ctrl |= PLL_16FFT_CTRL_PLL_EN;
	writel(ctrl, pll->base + PLL_16FFT_CTRL);

	/* Wait 1us */
	udelay(1);

	return 0;
}

static bool clk_pll_16fft_check_lock(const struct ti_pll_clk *pll)
{
	u32 stat;

	stat = readl(pll->base + PLL_16FFT_STAT);
	return (stat & PLL_16FFT_STAT_LOCK);
}

static bool clk_pll_16fft_check_cal_lock(const struct ti_pll_clk *pll)
{
	u32 stat;

	stat = readl(pll->base + PLL_16FFT_CAL_STAT);
	return (stat & PLL_16FFT_CAL_STAT_CAL_LOCK);
}

static void clk_pll_16fft_cal_int(const struct ti_pll_clk *pll)
{
	u32 cal;

	cal = readl(pll->base + PLL_16FFT_CAL_CTRL);

	/* Enable fast cal mode */
	cal |= PLL_16FFT_CAL_CTRL_FAST_CAL;

	/* Disable calibration bypass */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_BYP;

	/* Set CALCNT to 2 */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_CNT_MASK;
	cal |= 2 << PLL_16FFT_CAL_CTRL_CAL_CNT_SHIFT;

	/* Set CAL_IN to 0 */
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_IN_MASK;

	/* Note this register does not readback the written value. */
	writel(cal, pll->base + PLL_16FFT_CAL_CTRL);

	/* Wait 1us before enabling the CAL_EN field */
	udelay(1);

	cal = readl(pll->base + PLL_16FFT_CAL_CTRL);

	/* Enable calibration for FRACF */
	cal |= PLL_16FFT_CAL_CTRL_CAL_EN;

	/* Note this register does not readback the written value. */
	writel(cal, pll->base + PLL_16FFT_CAL_CTRL);
}

static void clk_pll_16fft_disable_cal(const struct ti_pll_clk *pll)
{
	u32 cal, stat;

	cal = readl(pll->base + PLL_16FFT_CAL_CTRL);
	cal &= ~PLL_16FFT_CAL_CTRL_CAL_EN;
	/* Note this register does not readback the written value. */
	writel(cal, pll->base + PLL_16FFT_CAL_CTRL);
	do {
		stat = readl(pll->base + PLL_16FFT_CAL_STAT);
	} while (stat & PLL_16FFT_CAL_STAT_CAL_LOCK);
}

static int ti_pll_wait_for_lock(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u32 cfg;
	u32 cal;
	u32 freq_ctrl1;
	unsigned int i;
	u32 pllfm;
	u32 pll_type;
	u32 cal_en = 0;
	bool success;

	/*
	 * Minimum VCO input freq is 5MHz, and the longest a lock should
	 * be consider to be timed out after 750 cycles. Be conservative
	 * and assume each loop takes 10 cycles and we run at a
	 * max of 1GHz. That gives 15000 loop cycles. We may end up waiting
	 * longer than necessary for timeout, but that should be ok.
	 */
	success = false;
	for (i = 0; i < PLL_16FFT_STAT_LOCK_TIMEOUT; i++) {
		if (clk_pll_16fft_check_lock(pll)) {
			success = true;
			break;
		}
	}

	/* Disable calibration in the fractional mode of the FRACF PLL based on data
	 * from silicon and simulation data.
	 */
	freq_ctrl1 = readl(pll->base + PLL_16FFT_FREQ_CTRL1);
	pllfm = freq_ctrl1 & PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;

	cfg = readl(pll->base + PLL_16FFT_CFG);
	pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

	if (success && pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) {
		cal = readl(pll->base + PLL_16FFT_CAL_CTRL);
		cal_en = (cal & PLL_16FFT_CAL_CTRL_CAL_EN);
	}

	if (success && pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF &&
	    pllfm == 0 && cal_en == 1) {
		/*
		 * Wait for calibration lock.
		 *
		 * Lock should occur within:
		 *
		 *	170 * 2^(5+CALCNT) / PFD
		 *      21760 / PFD
		 *
		 * CALCNT = 2, PFD = 5-50MHz. This gives a range of 0.435mS to
		 * 4.35mS depending on PFD frequency.
		 *
		 * Be conservative and assume each loop takes 10 cycles and we run at a
		 * max of 1GHz. That gives 435000 loop cycles. We may end up waiting
		 * longer than necessary for timeout, but that should be ok.
		 *
		 * The recommend timeout for CALLOCK to go high is 4.35 ms
		 */
		success = false;
		for (i = 0; i < PLL_16FFT_CAL_STAT_CAL_LOCK_TIMEOUT; i++) {
			if (clk_pll_16fft_check_cal_lock(pll)) {
				success = true;
				break;
			}
		}

		/* In case of cal lock failure, operate without calibration */
		if (!success) {
			debug("Failure for calibration, falling back without calibration\n");

			/* Disable PLL */
			ti_pll_clk_disable(clk);

			/* Disable Calibration */
			clk_pll_16fft_disable_cal(pll);

			/* Enable PLL */
			ti_pll_clk_enable(clk);

			/* Wait for PLL Lock */
			for (i = 0; i < PLL_16FFT_STAT_LOCK_TIMEOUT; i++) {
				if (clk_pll_16fft_check_lock(pll)) {
					success = true;
					break;
				}
			}
		}
	}

	if (!success) {
		printf("%s: pll (%s) failed to lock\n", __func__,
		       clk->dev->name);
		return -EBUSY;
	} else {
		return 0;
	}
}

static ulong ti_pll_clk_get_rate(struct clk *clk)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u64 current_freq;
	u64 parent_freq = clk_get_parent_rate(clk);
	u32 pllm;
	u32 plld;
	u32 pllfm;
	u32 ctrl;

	/* Check if we are in bypass */
	ctrl = readl(pll->base + PLL_16FFT_CTRL);
	if (ctrl & PLL_16FFT_CTRL_BYPASS_EN)
		return parent_freq;

	pllm = readl(pll->base + PLL_16FFT_FREQ_CTRL0);
	pllfm = readl(pll->base + PLL_16FFT_FREQ_CTRL1);

	plld = readl(pll->base + PLL_16FFT_DIV_CTRL) &
		PLL_16FFT_DIV_CTRL_REF_DIV_MASK;

	current_freq = parent_freq * pllm / plld;

	if (pllfm) {
		u64 tmp;

		tmp = parent_freq * pllfm;
		do_div(tmp, plld);
		tmp >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
		current_freq += tmp;
	}

	return current_freq;
}

static bool ti_pll_clk_is_bypass(struct ti_pll_clk *pll)
{
	u32 ctrl;
	bool ret;

	ctrl = readl(pll->base + PLL_16FFT_CTRL);
	ret = (ctrl & PLL_16FFT_CTRL_BYPASS_EN) != 0;

	return ret;
}

static void ti_pll_clk_bypass(struct ti_pll_clk *pll, bool bypass)
{
	u32 ctrl;

	ctrl = readl(pll->base + PLL_16FFT_CTRL);
	if (bypass)
		ctrl |= PLL_16FFT_CTRL_BYPASS_EN;
	else
		ctrl &= ~PLL_16FFT_CTRL_BYPASS_EN;

	writel(ctrl, pll->base + PLL_16FFT_CTRL);
}

static ulong ti_pll_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ti_pll_clk *pll = to_clk_pll(clk);
	u64 current_freq;
	u64 parent_freq = clk_get_parent_rate(clk);
	int ret;
	u32 ctrl;
	u32 cfg;
	u32 pll_type;
	unsigned long pllm;
	u32 pllfm = 0;
	unsigned long plld;
	u32 freq_ctrl0;
	u32 freq_ctrl1;
	u32 div_ctrl;
	u32 rem;
	int shift;

	debug("%s(clk=%p, rate=%u)\n", __func__, clk, (u32)rate);

	if (ti_pll_clk_get_rate(clk) == rate)
		return rate;

	if (rate != parent_freq)
		/*
		 * Attempt with higher max multiplier value first to give
		 * some space for fractional divider to kick in.
		 */
		for (shift = 8; shift >= 0; shift -= 8) {
			rational_best_approximation(rate, parent_freq,
				((PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK + 1) << shift) - 1,
				PLL_16FFT_DIV_CTRL_REF_DIV_MASK, &pllm, &plld);
			if (pllm / plld <= PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK)
				break;
		}

	if (!ti_pll_clk_is_bypass(pll)) {
		/* Put the PLL into bypass */
		ti_pll_clk_bypass(pll, true);
	}

	/* Disable the PLL */
	ti_pll_clk_disable(clk);

	if (rate == parent_freq) {
		debug("%s: put %s to bypass\n", __func__, clk->dev->name);
		return rate;
	}

	cfg = readl(pll->base + PLL_16FFT_CFG);
	pll_type = (cfg & PLL_16FFT_CFG_PLL_TYPE_MASK) >> PLL_16FFT_CFG_PLL_TYPE_SHIFT;

	debug("%s: pre-frac-calc: rate=%u, parent_freq=%u, plld=%u, pllm=%u\n",
	      __func__, (u32)rate, (u32)parent_freq, (u32)plld, (u32)pllm);

	/* Check if we need fractional config */
	if (plld > 1) {
		pllfm = pllm % plld;
		pllfm <<= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
		rem = pllfm % plld;
		pllfm /= plld;
		if (rem)
			pllfm++;
		pllm /= plld;
		plld = 1;
	}

	/* Program the new rate */
	freq_ctrl0 = readl(pll->base + PLL_16FFT_FREQ_CTRL0);
	freq_ctrl1 = readl(pll->base + PLL_16FFT_FREQ_CTRL1);
	div_ctrl = readl(pll->base + PLL_16FFT_DIV_CTRL);

	freq_ctrl0 &= ~PLL_16FFT_FREQ_CTRL0_FB_DIV_INT_MASK;
	freq_ctrl0 |= pllm;

	freq_ctrl1 &= ~PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_MASK;
	freq_ctrl1 |= pllfm;

	/*
	 * div_ctrl register contains other divider values, so rmw
	 * only plld and leave existing values alone
	 */
	div_ctrl &= ~PLL_16FFT_DIV_CTRL_REF_DIV_MASK;
	div_ctrl |= plld;

	/* Make sure we have fractional support if required */
	ctrl = readl(pll->base + PLL_16FFT_CTRL);

	/* Don't use internal bypass,it is not glitch free. Always prefer glitchless bypass */
	ctrl &= ~(PLL_16FFT_CTRL_INTL_BYP_EN | PLL_16FFT_CTRL_CLK_4PH_EN);

	/* Always enable output if PLL,  Always bypass if we lose lock */
	ctrl |= (PLL_16FFT_CTRL_CLK_POSTDIV_EN | PLL_16FFT_CTRL_BYP_ON_LOCKLOSS);

	/* Enable fractional support if required */
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) {
		if (pllfm != 0)
			ctrl |= (PLL_16FFT_CTRL_DSM_EN | PLL_16FFT_CTRL_DAC_EN);
		else
			ctrl &= ~(PLL_16FFT_CTRL_DSM_EN | PLL_16FFT_CTRL_DAC_EN);
	}

	/* Enable Fractional by default for PLL_16FFT_CFG_PLL_TYPE_FRAC2 */
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRAC2)
		ctrl |= (PLL_16FFT_CTRL_DSM_EN | PLL_16FFT_CTRL_DAC_EN);

	writel(freq_ctrl0, pll->base + PLL_16FFT_FREQ_CTRL0);
	writel(freq_ctrl1, pll->base + PLL_16FFT_FREQ_CTRL1);
	writel(div_ctrl, pll->base + PLL_16FFT_DIV_CTRL);
	writel(ctrl, pll->base + PLL_16FFT_CTRL);

	/* Configure PLL calibration*/
	if (pll_type == PLL_16FFT_CFG_PLL_TYPE_FRACF) {
		if (pllfm != 0) {
			/* Disable Calibration in Fractional mode */
			clk_pll_16fft_disable_cal(pll);
		} else {
			/* Enable Calibration in Integer mode */
			clk_pll_16fft_cal_int(pll);
		}
	}

	/*
	 * Wait at least 1 ref cycle before enabling PLL.
	 * Minimum VCO input frequency is 5MHz, therefore maximum
	 * wait time for 1 ref clock is 0.2us.
	 */
	udelay(1);
	ti_pll_clk_enable(clk);

	ret = ti_pll_wait_for_lock(clk);
	if (ret)
		return ret;

	ti_pll_clk_bypass(pll, false);

	debug("%s: pllm=%u, plld=%u, pllfm=%u, parent_freq=%u\n",
	      __func__, (u32)pllm, (u32)plld, (u32)pllfm, (u32)parent_freq);

	current_freq = parent_freq * pllm / plld;

	if (pllfm) {
		u64 tmp;

		tmp = parent_freq * pllfm;
		do_div(tmp, plld);
		tmp >>= PLL_16FFT_FREQ_CTRL1_FB_DIV_FRAC_BITS;
		current_freq += tmp;
	}

	return current_freq;
}



static const struct clk_ops ti_pll_clk_ops = {
	.get_rate = ti_pll_clk_get_rate,
	.set_rate = ti_pll_clk_set_rate,
	.enable = ti_pll_clk_enable,
	.disable = ti_pll_clk_disable,
};

struct clk *clk_register_ti_pll(const char *name, const char *parent_name,
				void __iomem *reg)
{
	struct ti_pll_clk *pll;
	int ret;
	int i;
	u32 cfg, ctrl, hsdiv_presence_bit, hsdiv_ctrl_offs;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);

	pll->base = reg;

	ret = clk_register(&pll->clk, "ti-pll-clk", name, parent_name);
	if (ret) {
		printf("%s: failed to register: %d\n", __func__, ret);
		kfree(pll);
		return ERR_PTR(ret);
	}

	/* Unlock the PLL registers */
	writel(PLL_KICK0_VALUE, pll->base + PLL_KICK0);
	writel(PLL_KICK1_VALUE, pll->base + PLL_KICK1);

	/* Enable all HSDIV outputs */
	cfg = readl(pll->base + PLL_16FFT_CFG);
	for (i = 0; i < 16; i++) {
		hsdiv_presence_bit = BIT(16 + i);
		hsdiv_ctrl_offs = 0x80 + (i * 4);
		/* Enable HSDIV output if present */
		if ((hsdiv_presence_bit & cfg) != 0UL) {
			ctrl = readl(pll->base + hsdiv_ctrl_offs);
			ctrl |= PLL_16FFT_HSDIV_CTRL_CLKOUT_EN;
			writel(ctrl, pll->base + hsdiv_ctrl_offs);
		}
	}

	return &pll->clk;
}

U_BOOT_DRIVER(ti_pll_clk) = {
	.name = "ti-pll-clk",
	.id = UCLASS_CLK,
	.ops = &ti_pll_clk_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

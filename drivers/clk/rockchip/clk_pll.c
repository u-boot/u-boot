// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2018-2019 Rockchip Electronics Co., Ltd
 */
 #include <common.h>
#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <asm/io.h>
#include <asm/arch-rockchip/clock.h>
#include <asm/arch-rockchip/hardware.h>
#include <div64.h>
#include <linux/delay.h>

static struct rockchip_pll_rate_table rockchip_auto_table;

#define PLL_MODE_MASK				0x3
#define PLL_RK3328_MODE_MASK			0x1

#define RK3036_PLLCON0_FBDIV_MASK		0xfff
#define RK3036_PLLCON0_FBDIV_SHIFT		0
#define RK3036_PLLCON0_POSTDIV1_MASK		0x7 << 12
#define RK3036_PLLCON0_POSTDIV1_SHIFT		12
#define RK3036_PLLCON1_REFDIV_MASK		0x3f
#define RK3036_PLLCON1_REFDIV_SHIFT		0
#define RK3036_PLLCON1_POSTDIV2_MASK		0x7 << 6
#define RK3036_PLLCON1_POSTDIV2_SHIFT		6
#define RK3036_PLLCON1_DSMPD_MASK		0x1 << 12
#define RK3036_PLLCON1_DSMPD_SHIFT		12
#define RK3036_PLLCON2_FRAC_MASK		0xffffff
#define RK3036_PLLCON2_FRAC_SHIFT		0
#define RK3036_PLLCON1_PWRDOWN_SHIT		13

#define MHZ		1000000
#define KHZ		1000
enum {
	OSC_HZ			= 24 * 1000000,
	VCO_MAX_HZ	= 3200U * 1000000,
	VCO_MIN_HZ	= 800 * 1000000,
	OUTPUT_MAX_HZ	= 3200U * 1000000,
	OUTPUT_MIN_HZ	= 24 * 1000000,
};

#define MIN_FOUTVCO_FREQ	(800 * MHZ)
#define MAX_FOUTVCO_FREQ	(2000 * MHZ)

int gcd(int m, int n)
{
	int t;

	while (m > 0) {
		if (n > m) {
			t = m;
			m = n;
			n = t;
		} /* swap */
		m -= n;
	}
	return n;
}

/*
 * How to calculate the PLL(from TRM V0.3 Part 1 Page 63):
 * Formulas also embedded within the Fractional PLL Verilog model:
 * If DSMPD = 1 (DSM is disabled, "integer mode")
 * FOUTVCO = FREF / REFDIV * FBDIV
 * FOUTPOSTDIV = FOUTVCO / POSTDIV1 / POSTDIV2
 * Where:
 * FOUTVCO = Fractional PLL non-divided output frequency
 * FOUTPOSTDIV = Fractional PLL divided output frequency
 *               (output of second post divider)
 * FREF = Fractional PLL input reference frequency, (the OSC_HZ 24MHz input)
 * REFDIV = Fractional PLL input reference clock divider
 * FBDIV = Integer value programmed into feedback divide
 *
 */

static int rockchip_pll_clk_set_postdiv(ulong fout_hz,
					u32 *postdiv1,
					u32 *postdiv2,
					u32 *foutvco)
{
	ulong freq;

	if (fout_hz < MIN_FOUTVCO_FREQ) {
		for (*postdiv1 = 1; *postdiv1 <= 7; (*postdiv1)++) {
			for (*postdiv2 = 1; *postdiv2 <= 7; (*postdiv2)++) {
				freq = fout_hz * (*postdiv1) * (*postdiv2);
				if (freq >= MIN_FOUTVCO_FREQ &&
				    freq <= MAX_FOUTVCO_FREQ) {
					*foutvco = freq;
					return 0;
				}
			}
		}
		printf("Can't FIND postdiv1/2 to make fout=%lu in 800~2000M.\n",
		       fout_hz);
	} else {
		*postdiv1 = 1;
		*postdiv2 = 1;
	}
	return 0;
}

static struct rockchip_pll_rate_table *
rockchip_pll_clk_set_by_auto(ulong fin_hz,
			     ulong fout_hz)
{
	struct rockchip_pll_rate_table *rate_table = &rockchip_auto_table;
	/* FIXME set postdiv1/2 always 1*/
	u32 foutvco = fout_hz;
	ulong fin_64, frac_64;
	u32 f_frac, postdiv1, postdiv2;
	ulong clk_gcd = 0;

	if (fin_hz == 0 || fout_hz == 0 || fout_hz == fin_hz)
		return NULL;

	rockchip_pll_clk_set_postdiv(fout_hz, &postdiv1, &postdiv2, &foutvco);
	rate_table->postdiv1 = postdiv1;
	rate_table->postdiv2 = postdiv2;
	rate_table->dsmpd = 1;

	if (fin_hz / MHZ * MHZ == fin_hz && fout_hz / MHZ * MHZ == fout_hz) {
		fin_hz /= MHZ;
		foutvco /= MHZ;
		clk_gcd = gcd(fin_hz, foutvco);
		rate_table->refdiv = fin_hz / clk_gcd;
		rate_table->fbdiv = foutvco / clk_gcd;

		rate_table->frac = 0;

		debug("fin = %ld, fout = %ld, clk_gcd = %ld,\n",
		      fin_hz, fout_hz, clk_gcd);
		debug("refdiv= %d,fbdiv= %d,postdiv1= %d,postdiv2= %d\n",
		      rate_table->refdiv,
		      rate_table->fbdiv, rate_table->postdiv1,
		      rate_table->postdiv2);
	} else {
		debug("frac div,fin_hz = %ld,fout_hz = %ld\n",
		      fin_hz, fout_hz);
		debug("frac get postdiv1 = %d,  postdiv2 = %d, foutvco = %d\n",
		      rate_table->postdiv1, rate_table->postdiv2, foutvco);
		clk_gcd = gcd(fin_hz / MHZ, foutvco / MHZ);
		rate_table->refdiv = fin_hz / MHZ / clk_gcd;
		rate_table->fbdiv = foutvco / MHZ / clk_gcd;
		debug("frac get refdiv = %d,  fbdiv = %d\n",
		      rate_table->refdiv, rate_table->fbdiv);

		rate_table->frac = 0;

		f_frac = (foutvco % MHZ);
		fin_64 = fin_hz;
		fin_64 = fin_64 / rate_table->refdiv;
		frac_64 = f_frac << 24;
		frac_64 = frac_64 / fin_64;
		rate_table->frac = frac_64;
		if (rate_table->frac > 0)
			rate_table->dsmpd = 0;
		debug("frac = %x\n", rate_table->frac);
	}
	return rate_table;
}

static const struct rockchip_pll_rate_table *
rockchip_get_pll_settings(struct rockchip_pll_clock *pll, ulong rate)
{
	struct rockchip_pll_rate_table  *rate_table = pll->rate_table;

	while (rate_table->rate) {
		if (rate_table->rate == rate)
			break;
		rate_table++;
	}
	if (rate_table->rate != rate)
		return rockchip_pll_clk_set_by_auto(24 * MHZ, rate);
	else
		return rate_table;
}

static int rk3036_pll_set_rate(struct rockchip_pll_clock *pll,
			       void __iomem *base, ulong pll_id,
			       ulong drate)
{
	const struct rockchip_pll_rate_table *rate;

	rate = rockchip_get_pll_settings(pll, drate);
	if (!rate) {
		printf("%s unsupport rate\n", __func__);
		return -EINVAL;
	}

	debug("%s: rate settings for %lu fbdiv: %d, postdiv1: %d, refdiv: %d\n",
	      __func__, rate->rate, rate->fbdiv, rate->postdiv1, rate->refdiv);
	debug("%s: rate settings for %lu postdiv2: %d, dsmpd: %d, frac: %d\n",
	      __func__, rate->rate, rate->postdiv2, rate->dsmpd, rate->frac);

	/*
	 * When power on or changing PLL setting,
	 * we must force PLL into slow mode to ensure output stable clock.
	 */
	rk_clrsetreg(base + pll->mode_offset,
		     pll->mode_mask << pll->mode_shift,
		     RKCLK_PLL_MODE_SLOW << pll->mode_shift);

	/* Power down */
	rk_setreg(base + pll->con_offset + 0x4,
		  1 << RK3036_PLLCON1_PWRDOWN_SHIT);

	rk_clrsetreg(base + pll->con_offset,
		     (RK3036_PLLCON0_POSTDIV1_MASK |
		     RK3036_PLLCON0_FBDIV_MASK),
		     (rate->postdiv1 << RK3036_PLLCON0_POSTDIV1_SHIFT) |
		     rate->fbdiv);
	rk_clrsetreg(base + pll->con_offset + 0x4,
		     (RK3036_PLLCON1_POSTDIV2_MASK |
		     RK3036_PLLCON1_REFDIV_MASK),
		     (rate->postdiv2 << RK3036_PLLCON1_POSTDIV2_SHIFT |
		     rate->refdiv << RK3036_PLLCON1_REFDIV_SHIFT));
	if (!rate->dsmpd) {
		rk_clrsetreg(base + pll->con_offset + 0x4,
			     RK3036_PLLCON1_DSMPD_MASK,
			     rate->dsmpd << RK3036_PLLCON1_DSMPD_SHIFT);
		writel((readl(base + pll->con_offset + 0x8) &
			(~RK3036_PLLCON2_FRAC_MASK)) |
			    (rate->frac << RK3036_PLLCON2_FRAC_SHIFT),
			    base + pll->con_offset + 0x8);
	}

	/* Power Up */
	rk_clrreg(base + pll->con_offset + 0x4,
		  1 << RK3036_PLLCON1_PWRDOWN_SHIT);

	/* waiting for pll lock */
	while (!(readl(base + pll->con_offset + 0x4) & (1 << pll->lock_shift)))
		udelay(1);

	rk_clrsetreg(base + pll->mode_offset, pll->mode_mask << pll->mode_shift,
		     RKCLK_PLL_MODE_NORMAL << pll->mode_shift);
	debug("PLL at %p: con0=%x con1= %x con2= %x mode= %x\n",
	      pll, readl(base + pll->con_offset),
	      readl(base + pll->con_offset + 0x4),
	      readl(base + pll->con_offset + 0x8),
	      readl(base + pll->mode_offset));

	return 0;
}

static ulong rk3036_pll_get_rate(struct rockchip_pll_clock *pll,
				 void __iomem *base, ulong pll_id)
{
	u32 refdiv, fbdiv, postdiv1, postdiv2, dsmpd, frac;
	u32 con = 0, shift, mask;
	ulong rate;

	con = readl(base + pll->mode_offset);
	shift = pll->mode_shift;
	mask = pll->mode_mask << shift;

	switch ((con & mask) >> shift) {
	case RKCLK_PLL_MODE_SLOW:
		return OSC_HZ;
	case RKCLK_PLL_MODE_NORMAL:
		/* normal mode */
		con = readl(base + pll->con_offset);
		postdiv1 = (con & RK3036_PLLCON0_POSTDIV1_MASK) >>
			   RK3036_PLLCON0_POSTDIV1_SHIFT;
		fbdiv = (con & RK3036_PLLCON0_FBDIV_MASK) >>
			RK3036_PLLCON0_FBDIV_SHIFT;
		con = readl(base + pll->con_offset + 0x4);
		postdiv2 = (con & RK3036_PLLCON1_POSTDIV2_MASK) >>
			   RK3036_PLLCON1_POSTDIV2_SHIFT;
		refdiv = (con & RK3036_PLLCON1_REFDIV_MASK) >>
			 RK3036_PLLCON1_REFDIV_SHIFT;
		dsmpd = (con & RK3036_PLLCON1_DSMPD_MASK) >>
			RK3036_PLLCON1_DSMPD_SHIFT;
		con = readl(base + pll->con_offset + 0x8);
		frac = (con & RK3036_PLLCON2_FRAC_MASK) >>
			RK3036_PLLCON2_FRAC_SHIFT;
		rate = (24 * fbdiv / (refdiv * postdiv1 * postdiv2)) * 1000000;
		if (dsmpd == 0) {
			u64 frac_rate = OSC_HZ * (u64)frac;

			do_div(frac_rate, refdiv);
			frac_rate >>= 24;
			do_div(frac_rate, postdiv1);
			do_div(frac_rate, postdiv1);
			rate += frac_rate;
		}
		return rate;
	case RKCLK_PLL_MODE_DEEP:
	default:
		return 32768;
	}
}

ulong rockchip_pll_get_rate(struct rockchip_pll_clock *pll,
			    void __iomem *base,
			    ulong pll_id)
{
	ulong rate = 0;

	switch (pll->type) {
	case pll_rk3036:
		pll->mode_mask = PLL_MODE_MASK;
		rate = rk3036_pll_get_rate(pll, base, pll_id);
		break;
	case pll_rk3328:
		pll->mode_mask = PLL_RK3328_MODE_MASK;
		rate = rk3036_pll_get_rate(pll, base, pll_id);
		break;
	default:
		printf("%s: Unknown pll type for pll clk %ld\n",
		       __func__, pll_id);
	}
	return rate;
}

int rockchip_pll_set_rate(struct rockchip_pll_clock *pll,
			  void __iomem *base, ulong pll_id,
			  ulong drate)
{
	int ret = 0;

	if (rockchip_pll_get_rate(pll, base, pll_id) == drate)
		return 0;

	switch (pll->type) {
	case pll_rk3036:
		pll->mode_mask = PLL_MODE_MASK;
		ret = rk3036_pll_set_rate(pll, base, pll_id, drate);
		break;
	case pll_rk3328:
		pll->mode_mask = PLL_RK3328_MODE_MASK;
		ret = rk3036_pll_set_rate(pll, base, pll_id, drate);
		break;
	default:
		printf("%s: Unknown pll type for pll clk %ld\n",
		       __func__, pll_id);
	}
	return ret;
}

const struct rockchip_cpu_rate_table *
rockchip_get_cpu_settings(struct rockchip_cpu_rate_table *cpu_table,
			  ulong rate)
{
	struct rockchip_cpu_rate_table *ps = cpu_table;

	while (ps->rate) {
		if (ps->rate == rate)
			break;
		ps++;
	}
	if (ps->rate != rate)
		return NULL;
	else
		return ps;
}


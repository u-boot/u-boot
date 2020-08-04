// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-20 Sean Anderson <seanga2@gmail.com>
 */
#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <dm.h>
/* For DIV_ROUND_DOWN_ULL, defined in linux/kernel.h */
#include <div64.h>
#include <log.h>
#include <serial.h>
#include <asm/io.h>
#include <dt-bindings/clock/k210-sysctl.h>
#include <kendryte/pll.h>
#include <linux/bitfield.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/err.h>

#define CLK_K210_PLL "k210_clk_pll"

#ifdef CONFIG_CLK_K210_SET_RATE
static int k210_pll_enable(struct clk *clk);
static int k210_pll_disable(struct clk *clk);

/*
 * The PLL included with the Kendryte K210 appears to be a True Circuits, Inc.
 * General-Purpose PLL. The logical layout of the PLL with internal feedback is
 * approximately the following:
 *
 *  +---------------+
 *  |reference clock|
 *  +---------------+
 *          |
 *          v
 *        +--+
 *        |/r|
 *        +--+
 *          |
 *          v
 *   +-------------+
 *   |divided clock|
 *   +-------------+
 *          |
 *          v
 *  +--------------+
 *  |phase detector|<---+
 *  +--------------+    |
 *          |           |
 *          v   +--------------+
 *        +---+ |feedback clock|
 *        |VCO| +--------------+
 *        +---+         ^
 *          |    +--+   |
 *          +--->|/f|---+
 *          |    +--+
 *          v
 *        +---+
 *        |/od|
 *        +---+
 *          |
 *          v
 *       +------+
 *       |output|
 *       +------+
 *
 * The k210 PLLs have three factors: r, f, and od. Because of the feedback mode,
 * the effect of the division by f is to multiply the input frequency. The
 * equation for the output rate is
 *   rate = (rate_in * f) / (r * od).
 * Moving knowns to one side of the equation, we get
 *   rate / rate_in = f / (r * od)
 * Rearranging slightly,
 *   abs_error = abs((rate / rate_in) - (f / (r * od))).
 * To get relative, error, we divide by the expected ratio
 *   error = abs((rate / rate_in) - (f / (r * od))) / (rate / rate_in).
 * Simplifying,
 *   error = abs(1 - f / (r * od)) / (rate / rate_in)
 *   error = abs(1 - (f * rate_in) / (r * od * rate))
 * Using the constants ratio = rate / rate_in and inv_ratio = rate_in / rate,
 *   error = abs((f * inv_ratio) / (r * od) - 1)
 * This is the error used in evaluating parameters.
 *
 * r and od are four bits each, while f is six bits. Because r and od are
 * multiplied together, instead of the full 256 values possible if both bits
 * were used fully, there are only 97 distinct products. Combined with f, there
 * are 6208 theoretical settings for the PLL. However, most of these settings
 * can be ruled out immediately because they do not have the correct ratio.
 *
 * In addition to the constraint of approximating the desired ratio, parameters
 * must also keep internal pll frequencies within acceptable ranges. The divided
 * clock's minimum and maximum frequencies have a ratio of around 128.  This
 * leaves fairly substantial room to work with, especially since the only
 * affected parameter is r. The VCO's minimum and maximum frequency have a ratio
 * of 5, which is considerably more restrictive.
 *
 * The r and od factors are stored in a table. This is to make it easy to find
 * the next-largest product. Some products have multiple factorizations, but
 * only when one factor has at least a 2.5x ratio to the factors of the other
 * factorization. This is because any smaller ratio would not make a difference
 * when ensuring the VCO's frequency is within spec.
 *
 * Throughout the calculation function, fixed point arithmetic is used. Because
 * the range of rate and rate_in may be up to 1.75 GHz, or around 2^30, 64-bit
 * 32.32 fixed-point numbers are used to represent ratios. In general, to
 * implement division, the numerator is first multiplied by 2^32. This gives a
 * result where the whole number part is in the upper 32 bits, and the fraction
 * is in the lower 32 bits.
 *
 * In general, rounding is done to the closest integer. This helps find the best
 * approximation for the ratio. Rounding in one direction (e.g down) could cause
 * the function to miss a better ratio with one of the parameters increased by
 * one.
 */

/*
 * The factors table was generated with the following python code:
 *
 * def p(x, y):
 *    return (1.0*x/y > 2.5) or (1.0*y/x > 2.5)
 *
 * factors = {}
 * for i in range(1, 17):
 *    for j in range(1, 17):
 *       fs = factors.get(i*j) or []
 *       if fs == [] or all([
 *             (p(i, x) and p(i, y)) or (p(j, x) and p(j, y))
 *             for (x, y) in fs]):
 *          fs.append((i, j))
 *          factors[i*j] = fs
 *
 * for k, l in sorted(factors.items()):
 *    for v in l:
 *       print("PACK(%s, %s)," % v)
 */
#define PACK(r, od) (((((r) - 1) & 0xF) << 4) | (((od) - 1) & 0xF))
#define UNPACK_R(val) ((((val) >> 4) & 0xF) + 1)
#define UNPACK_OD(val) (((val) & 0xF) + 1)
static const u8 factors[] = {
	PACK(1, 1),
	PACK(1, 2),
	PACK(1, 3),
	PACK(1, 4),
	PACK(1, 5),
	PACK(1, 6),
	PACK(1, 7),
	PACK(1, 8),
	PACK(1, 9),
	PACK(3, 3),
	PACK(1, 10),
	PACK(1, 11),
	PACK(1, 12),
	PACK(3, 4),
	PACK(1, 13),
	PACK(1, 14),
	PACK(1, 15),
	PACK(3, 5),
	PACK(1, 16),
	PACK(4, 4),
	PACK(2, 9),
	PACK(2, 10),
	PACK(3, 7),
	PACK(2, 11),
	PACK(2, 12),
	PACK(5, 5),
	PACK(2, 13),
	PACK(3, 9),
	PACK(2, 14),
	PACK(2, 15),
	PACK(2, 16),
	PACK(3, 11),
	PACK(5, 7),
	PACK(3, 12),
	PACK(3, 13),
	PACK(4, 10),
	PACK(3, 14),
	PACK(4, 11),
	PACK(3, 15),
	PACK(3, 16),
	PACK(7, 7),
	PACK(5, 10),
	PACK(4, 13),
	PACK(6, 9),
	PACK(5, 11),
	PACK(4, 14),
	PACK(4, 15),
	PACK(7, 9),
	PACK(4, 16),
	PACK(5, 13),
	PACK(6, 11),
	PACK(5, 14),
	PACK(6, 12),
	PACK(5, 15),
	PACK(7, 11),
	PACK(6, 13),
	PACK(5, 16),
	PACK(9, 9),
	PACK(6, 14),
	PACK(8, 11),
	PACK(6, 15),
	PACK(7, 13),
	PACK(6, 16),
	PACK(7, 14),
	PACK(9, 11),
	PACK(10, 10),
	PACK(8, 13),
	PACK(7, 15),
	PACK(9, 12),
	PACK(10, 11),
	PACK(7, 16),
	PACK(9, 13),
	PACK(8, 15),
	PACK(11, 11),
	PACK(9, 14),
	PACK(8, 16),
	PACK(10, 13),
	PACK(11, 12),
	PACK(9, 15),
	PACK(10, 14),
	PACK(11, 13),
	PACK(9, 16),
	PACK(10, 15),
	PACK(11, 14),
	PACK(12, 13),
	PACK(10, 16),
	PACK(11, 15),
	PACK(12, 14),
	PACK(13, 13),
	PACK(11, 16),
	PACK(12, 15),
	PACK(13, 14),
	PACK(12, 16),
	PACK(13, 15),
	PACK(14, 14),
	PACK(13, 16),
	PACK(14, 15),
	PACK(14, 16),
	PACK(15, 15),
	PACK(15, 16),
	PACK(16, 16),
};

TEST_STATIC int k210_pll_calc_config(u32 rate, u32 rate_in,
				     struct k210_pll_config *best)
{
	int i;
	s64 error, best_error;
	u64 ratio, inv_ratio; /* fixed point 32.32 ratio of the rates */
	u64 max_r;
	u64 r, f, od;

	/*
	 * Can't go over 1.75 GHz or under 21.25 MHz due to limitations on the
	 * VCO frequency. These are not the same limits as below because od can
	 * reduce the output frequency by 16.
	 */
	if (rate > 1750000000 || rate < 21250000)
		return -EINVAL;

	/* Similar restrictions on the input rate */
	if (rate_in > 1750000000 || rate_in < 13300000)
		return -EINVAL;

	ratio = DIV_ROUND_CLOSEST_ULL((u64)rate << 32, rate_in);
	inv_ratio = DIV_ROUND_CLOSEST_ULL((u64)rate_in << 32, rate);
	/* Can't increase by more than 64 or reduce by more than 256 */
	if (rate > rate_in && ratio > (64ULL << 32))
		return -EINVAL;
	else if (rate <= rate_in && inv_ratio > (256ULL << 32))
		return -EINVAL;

	/*
	 * The divided clock (rate_in / r) must stay between 1.75 GHz and 13.3
	 * MHz. There is no minimum, since the only way to get a higher input
	 * clock than 26 MHz is to use a clock generated by a PLL. Because PLLs
	 * cannot output frequencies greater than 1.75 GHz, the minimum would
	 * never be greater than one.
	 */
	max_r = DIV_ROUND_DOWN_ULL(rate_in, 13300000);

	/* Variables get immediately incremented, so start at -1th iteration */
	i = -1;
	f = 0;
	r = 0;
	od = 0;
	best_error = S64_MAX;
	error = best_error;
	/* do-while here so we always try at least one ratio */
	do {
		/*
		 * Whether we swapped r and od while enforcing frequency limits
		 */
		bool swapped = false;
		u64 last_od = od;
		u64 last_r = r;

		/*
		 * Try the next largest value for f (or r and od) and
		 * recalculate the other parameters based on that
		 */
		if (rate > rate_in) {
			/*
			 * Skip factors of the same product if we already tried
			 * out that product
			 */
			do {
				i++;
				r = UNPACK_R(factors[i]);
				od = UNPACK_OD(factors[i]);
			} while (i + 1 < ARRAY_SIZE(factors) &&
				 r * od == last_r * last_od);

			/* Round close */
			f = (r * od * ratio + BIT(31)) >> 32;
			if (f > 64)
				f = 64;
		} else {
			u64 tmp = ++f * inv_ratio;
			bool round_up = !!(tmp & BIT(31));
			u32 goal = (tmp >> 32) + round_up;
			u32 err, last_err;

			/* Get the next r/od pair in factors */
			while (r * od < goal && i + 1 < ARRAY_SIZE(factors)) {
				i++;
				r = UNPACK_R(factors[i]);
				od = UNPACK_OD(factors[i]);
			}

			/*
			 * This is a case of double rounding. If we rounded up
			 * above, we need to round down (in cases of ties) here.
			 * This prevents off-by-one errors resulting from
			 * choosing X+2 over X when X.Y rounds up to X+1 and
			 * there is no r * od = X+1. For the converse, when X.Y
			 * is rounded down to X, we should choose X+1 over X-1.
			 */
			err = abs(r * od - goal);
			last_err = abs(last_r * last_od - goal);
			if (last_err < err || (round_up && last_err == err)) {
				i--;
				r = last_r;
				od = last_od;
			}
		}

		/*
		 * Enforce limits on internal clock frequencies. If we
		 * aren't in spec, try swapping r and od. If everything is
		 * in-spec, calculate the relative error.
		 */
		while (true) {
			/*
			 * Whether the intermediate frequencies are out-of-spec
			 */
			bool out_of_spec = false;

			if (r > max_r) {
				out_of_spec = true;
			} else {
				/*
				 * There is no way to only divide once; we need
				 * to examine the frequency with and without the
				 * effect of od.
				 */
				u64 vco = DIV_ROUND_CLOSEST_ULL(rate_in * f, r);

				if (vco > 1750000000 || vco < 340000000)
					out_of_spec = true;
			}

			if (out_of_spec) {
				if (!swapped) {
					u64 tmp = r;

					r = od;
					od = tmp;
					swapped = true;
					continue;
				} else {
					/*
					 * Try looking ahead to see if there are
					 * additional factors for the same
					 * product.
					 */
					if (i + 1 < ARRAY_SIZE(factors)) {
						u64 new_r, new_od;

						i++;
						new_r = UNPACK_R(factors[i]);
						new_od = UNPACK_OD(factors[i]);
						if (r * od == new_r * new_od) {
							r = new_r;
							od = new_od;
							swapped = false;
							continue;
						}
						i--;
					}
					break;
				}
			}

			error = DIV_ROUND_CLOSEST_ULL(f * inv_ratio, r * od);
			/* The lower 16 bits are spurious */
			error = abs((error - BIT(32))) >> 16;

			if (error < best_error) {
				best->r = r;
				best->f = f;
				best->od = od;
				best_error = error;
			}
			break;
		}
	} while (f < 64 && i + 1 < ARRAY_SIZE(factors) && error != 0);

	if (best_error == S64_MAX)
		return -EINVAL;

	log_debug("best error %lld\n", best_error);
	return 0;
}

static ulong k210_pll_set_rate(struct clk *clk, ulong rate)
{
	int err;
	long long rate_in = clk_get_parent_rate(clk);
	struct k210_pll_config config = {};
	struct k210_pll *pll = to_k210_pll(clk);
	u32 reg;

	if (rate_in < 0)
		return rate_in;

	log_debug("Calculating parameters with rate=%lu and rate_in=%lld\n",
		  rate, rate_in);
	err = k210_pll_calc_config(rate, rate_in, &config);
	if (err)
		return err;
	log_debug("Got r=%u f=%u od=%u\n", config.r, config.f, config.od);

	/*
	 * Don't use clk_disable as it might not actually disable the pll due to
	 * refcounting
	 */
	k210_pll_disable(clk);

	reg = readl(pll->reg);
	reg &= ~K210_PLL_CLKR
	    &  ~K210_PLL_CLKF
	    &  ~K210_PLL_CLKOD
	    &  ~K210_PLL_BWADJ;
	reg |= FIELD_PREP(K210_PLL_CLKR, config.r - 1)
	    |  FIELD_PREP(K210_PLL_CLKF, config.f - 1)
	    |  FIELD_PREP(K210_PLL_CLKOD, config.od - 1)
	    |  FIELD_PREP(K210_PLL_BWADJ, config.f - 1);
	writel(reg, pll->reg);

	err = k210_pll_enable(clk);
	if (err)
		return err;

	serial_setbrg();
	return clk_get_rate(clk);
}
#endif /* CONFIG_CLK_K210_SET_RATE */

static ulong k210_pll_get_rate(struct clk *clk)
{
	long long rate_in = clk_get_parent_rate(clk);
	struct k210_pll *pll = to_k210_pll(clk);
	u64 r, f, od;
	u32 reg = readl(pll->reg);

	if (rate_in < 0 || (reg & K210_PLL_BYPASS))
		return rate_in;

	if (!(reg & K210_PLL_PWRD))
		return 0;

	r = FIELD_GET(K210_PLL_CLKR, reg) + 1;
	f = FIELD_GET(K210_PLL_CLKF, reg) + 1;
	od = FIELD_GET(K210_PLL_CLKOD, reg) + 1;

	return DIV_ROUND_DOWN_ULL(((u64)rate_in) * f, r * od);
}

/*
 * Wait for the PLL to be locked. If the PLL is not locked, try clearing the
 * slip before retrying
 */
static void k210_pll_waitfor_lock(struct k210_pll *pll)
{
	u32 mask = GENMASK(pll->width - 1, 0) << pll->shift;

	while (true) {
		u32 reg = readl(pll->lock);

		if ((reg & mask) == mask)
			break;

		reg |= BIT(pll->shift + K210_PLL_CLEAR_SLIP);
		writel(reg, pll->lock);
	}
}

/* Adapted from sysctl_pll_enable */
static int k210_pll_enable(struct clk *clk)
{
	struct k210_pll *pll = to_k210_pll(clk);
	u32 reg = readl(pll->reg);

	if ((reg | K210_PLL_PWRD) && !(reg | K210_PLL_RESET))
		return 0;

	reg |= K210_PLL_PWRD;
	writel(reg, pll->reg);

	/* Ensure reset is low before asserting it */
	reg &= ~K210_PLL_RESET;
	writel(reg, pll->reg);
	reg |= K210_PLL_RESET;
	writel(reg, pll->reg);
	nop();
	nop();
	reg &= ~K210_PLL_RESET;
	writel(reg, pll->reg);

	k210_pll_waitfor_lock(pll);

	reg &= ~K210_PLL_BYPASS;
	writel(reg, pll->reg);

	return 0;
}

static int k210_pll_disable(struct clk *clk)
{
	struct k210_pll *pll = to_k210_pll(clk);
	u32 reg = readl(pll->reg);

	/*
	 * Bypassing before powering off is important so child clocks don't stop
	 * working. This is especially important for pll0, the indirect parent
	 * of the cpu clock.
	 */
	reg |= K210_PLL_BYPASS;
	writel(reg, pll->reg);

	reg &= ~K210_PLL_PWRD;
	writel(reg, pll->reg);
	return 0;
}

const struct clk_ops k210_pll_ops = {
	.get_rate = k210_pll_get_rate,
#ifdef CONFIG_CLK_K210_SET_RATE
	.set_rate = k210_pll_set_rate,
#endif
	.enable = k210_pll_enable,
	.disable = k210_pll_disable,
};

struct clk *k210_register_pll_struct(const char *name, const char *parent_name,
				     struct k210_pll *pll)
{
	int ret;
	struct clk *clk = &pll->clk;

	ret = clk_register(clk, CLK_K210_PLL, name, parent_name);
	if (ret)
		return ERR_PTR(ret);
	return clk;
}

struct clk *k210_register_pll(const char *name, const char *parent_name,
			      void __iomem *reg, void __iomem *lock, u8 shift,
			      u8 width)
{
	struct clk *clk;
	struct k210_pll *pll;

	pll = kzalloc(sizeof(*pll), GFP_KERNEL);
	if (!pll)
		return ERR_PTR(-ENOMEM);
	pll->reg = reg;
	pll->lock = lock;
	pll->shift = shift;
	pll->width = width;

	clk = k210_register_pll_struct(name, parent_name, pll);
	if (IS_ERR(clk))
		kfree(pll);
	return clk;
}

U_BOOT_DRIVER(k210_pll) = {
	.name	= CLK_K210_PLL,
	.id	= UCLASS_CLK,
	.ops	= &k210_pll_ops,
};

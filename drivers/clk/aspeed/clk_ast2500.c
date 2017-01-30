/*
 * (C) Copyright 2016 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2500.h>
#include <dm/lists.h>
#include <dt-bindings/clock/ast2500-scu.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * For H-PLL and M-PLL the formula is
 * (Output Frequency) = CLKIN * ((M + 1) / (N + 1)) / (P + 1)
 * M - Numerator
 * N - Denumerator
 * P - Post Divider
 * They have the same layout in their control register.
 */

/*
 * Get the rate of the M-PLL clock from input clock frequency and
 * the value of the M-PLL Parameter Register.
 */
static ulong ast2500_get_mpll_rate(ulong clkin, u32 mpll_reg)
{
	const ulong num = (mpll_reg >> SCU_MPLL_NUM_SHIFT) & SCU_MPLL_NUM_MASK;
	const ulong denum = (mpll_reg >> SCU_MPLL_DENUM_SHIFT)
			& SCU_MPLL_DENUM_MASK;
	const ulong post_div = (mpll_reg >> SCU_MPLL_POST_SHIFT)
			& SCU_MPLL_POST_MASK;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}

/*
 * Get the rate of the H-PLL clock from input clock frequency and
 * the value of the H-PLL Parameter Register.
 */
static ulong ast2500_get_hpll_rate(ulong clkin, u32 hpll_reg)
{
	const ulong num = (hpll_reg >> SCU_HPLL_NUM_SHIFT) & SCU_HPLL_NUM_MASK;
	const ulong denum = (hpll_reg >> SCU_HPLL_DENUM_SHIFT)
			& SCU_HPLL_DENUM_MASK;
	const ulong post_div = (hpll_reg >> SCU_HPLL_POST_SHIFT)
			& SCU_HPLL_POST_MASK;

	return (clkin * ((num + 1) / (denum + 1))) / (post_div + 1);
}

static ulong ast2500_get_clkin(struct ast2500_scu *scu)
{
	return readl(&scu->hwstrap) & SCU_HWSTRAP_CLKIN_25MHZ
			? 25 * 1000 * 1000 : 24 * 1000 * 1000;
}

/**
 * Get current rate or uart clock
 *
 * @scu SCU registers
 * @uart_index UART index, 1-5
 *
 * @return current setting for uart clock rate
 */
static ulong ast2500_get_uart_clk_rate(struct ast2500_scu *scu, int uart_index)
{
	/*
	 * ast2500 datasheet is very confusing when it comes to UART clocks,
	 * especially when CLKIN = 25 MHz. The settings are in
	 * different registers and it is unclear how they interact.
	 *
	 * This has only been tested with default settings and CLKIN = 24 MHz.
	 */
	ulong uart_clkin;

	if (readl(&scu->misc_ctrl2) &
	    (1 << (uart_index - 1 + SCU_MISC2_UARTCLK_SHIFT)))
		uart_clkin = 192 * 1000 * 1000;
	else
		uart_clkin = 24 * 1000 * 1000;

	if (readl(&scu->misc_ctrl1) & SCU_MISC_UARTCLK_DIV13)
		uart_clkin /= 13;

	return uart_clkin;
}

static ulong ast2500_clk_get_rate(struct clk *clk)
{
	struct ast2500_clk_priv *priv = dev_get_priv(clk->dev);
	ulong clkin = ast2500_get_clkin(priv->scu);
	ulong rate;

	switch (clk->id) {
	case PLL_HPLL:
	case ARMCLK:
		/*
		 * This ignores dynamic/static slowdown of ARMCLK and may
		 * be inaccurate.
		 */
		rate = ast2500_get_hpll_rate(clkin,
					     readl(&priv->scu->h_pll_param));
		break;
	case MCLK_DDR:
		rate = ast2500_get_mpll_rate(clkin,
					     readl(&priv->scu->m_pll_param));
		break;
	case PCLK_UART1:
		rate = ast2500_get_uart_clk_rate(priv->scu, 1);
		break;
	case PCLK_UART2:
		rate = ast2500_get_uart_clk_rate(priv->scu, 2);
		break;
	case PCLK_UART3:
		rate = ast2500_get_uart_clk_rate(priv->scu, 3);
		break;
	case PCLK_UART4:
		rate = ast2500_get_uart_clk_rate(priv->scu, 4);
		break;
	case PCLK_UART5:
		rate = ast2500_get_uart_clk_rate(priv->scu, 5);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static void ast2500_scu_unlock(struct ast2500_scu *scu)
{
	writel(SCU_UNLOCK_VALUE, &scu->protection_key);
	while (!readl(&scu->protection_key))
		;
}

static void ast2500_scu_lock(struct ast2500_scu *scu)
{
	writel(~SCU_UNLOCK_VALUE, &scu->protection_key);
	while (readl(&scu->protection_key))
		;
}

static ulong ast2500_configure_ddr(struct ast2500_scu *scu, ulong rate)
{
	ulong clkin = ast2500_get_clkin(scu);
	u32 mpll_reg;

	/*
	 * There are not that many combinations of numerator, denumerator
	 * and post divider, so just brute force the best combination.
	 * However, to avoid overflow when multiplying, use kHz.
	 */
	const ulong clkin_khz = clkin / 1000;
	const ulong rate_khz = rate / 1000;
	ulong best_num = 0;
	ulong best_denum = 0;
	ulong best_post = 0;
	ulong delta = rate;
	ulong num, denum, post;

	for (denum = 0; denum <= SCU_MPLL_DENUM_MASK; ++denum) {
		for (post = 0; post <= SCU_MPLL_POST_MASK; ++post) {
			num = (rate_khz * (post + 1) / clkin_khz) * (denum + 1);
			ulong new_rate_khz = (clkin_khz
					      * ((num + 1) / (denum + 1)))
					     / (post + 1);

			/* Keep the rate below requested one. */
			if (new_rate_khz > rate_khz)
				continue;

			if (new_rate_khz - rate_khz < delta) {
				delta = new_rate_khz - rate_khz;

				best_num = num;
				best_denum = denum;
				best_post = post;

				if (delta == 0)
					goto rate_calc_done;
			}
		}
	}

 rate_calc_done:
	mpll_reg = readl(&scu->m_pll_param);
	mpll_reg &= ~((SCU_MPLL_POST_MASK << SCU_MPLL_POST_SHIFT)
		      | (SCU_MPLL_NUM_MASK << SCU_MPLL_NUM_SHIFT)
		      | (SCU_MPLL_DENUM_MASK << SCU_MPLL_DENUM_SHIFT));
	mpll_reg |= (best_post << SCU_MPLL_POST_SHIFT)
	    | (best_num << SCU_MPLL_NUM_SHIFT)
	    | (best_denum << SCU_MPLL_DENUM_SHIFT);

	ast2500_scu_unlock(scu);
	writel(mpll_reg, &scu->m_pll_param);
	ast2500_scu_lock(scu);

	return ast2500_get_mpll_rate(clkin, mpll_reg);
}

static ulong ast2500_clk_set_rate(struct clk *clk, ulong rate)
{
	struct ast2500_clk_priv *priv = dev_get_priv(clk->dev);

	ulong new_rate;
	switch (clk->id) {
	case PLL_MPLL:
	case MCLK_DDR:
		new_rate = ast2500_configure_ddr(priv->scu, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

struct clk_ops ast2500_clk_ops = {
	.get_rate = ast2500_clk_get_rate,
	.set_rate = ast2500_clk_set_rate,
};

static int ast2500_clk_probe(struct udevice *dev)
{
	struct ast2500_clk_priv *priv = dev_get_priv(dev);

	priv->scu = dev_get_addr_ptr(dev);
	if (IS_ERR(priv->scu))
		return PTR_ERR(priv->scu);

	return 0;
}

static int ast2500_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "ast_sysreset", "reset", &dev);
	if (ret)
		debug("Warning: No reset driver: ret=%d\n", ret);

	return 0;
}

static const struct udevice_id ast2500_clk_ids[] = {
	{ .compatible = "aspeed,ast2500-scu" },
	{ }
};

U_BOOT_DRIVER(aspeed_ast2500_scu) = {
	.name		= "aspeed_ast2500_scu",
	.id		= UCLASS_CLK,
	.of_match	= ast2500_clk_ids,
	.priv_auto_alloc_size = sizeof(struct ast2500_clk_priv),
	.ops		= &ast2500_clk_ops,
	.bind		= ast2500_clk_bind,
	.probe		= ast2500_clk_probe,
};

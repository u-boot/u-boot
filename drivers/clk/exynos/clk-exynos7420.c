// SPDX-License-Identifier: GPL-2.0+
/*
 * Samsung Exynos7420 clock driver.
 * Copyright (C) 2016 Samsung Electronics
 * Thomas Abraham <thomas.ab@samsung.com>
 */

#include <dm.h>
#include <errno.h>
#include <clk-uclass.h>
#include <asm/io.h>
#include <div64.h>
#include <dt-bindings/clock/exynos7420-clk.h>

#define PLL145X_MDIV_SHIFT	16
#define PLL145X_MDIV_MASK	0x3ff
#define PLL145X_PDIV_SHIFT	8
#define PLL145X_PDIV_MASK	0x3f
#define PLL145X_SDIV_SHIFT	0
#define PLL145X_SDIV_MASK	0x7

#define DIVIDER(reg, shift, mask)	\
	(((readl(reg) >> shift) & mask) + 1)

/* CMU TOPC block device structure */
struct exynos7420_clk_cmu_topc {
	unsigned int	rsvd1[68];
	unsigned int	bus0_pll_con[2];
	unsigned int	rsvd2[2];
	unsigned int	bus1_pll_con[2];
	unsigned int	rsvd3[54];
	unsigned int	mux_sel[6];
	unsigned int	rsvd4[250];
	unsigned int	div[4];
};

/* CMU TOP0 block device structure */
struct exynos7420_clk_cmu_top0 {
	unsigned int	rsvd0[128];
	unsigned int	mux_sel[7];
	unsigned int	rsvd1[261];
	unsigned int	div_peric[5];
};

/**
 * struct exynos7420_clk_topc_priv - private data for CMU topc clock driver.
 *
 * @topc: base address of the memory mapped CMU TOPC controller.
 * @fin_freq: frequency of the Oscillator clock.
 * @sclk_bus0_pll_a: frequency of sclk_bus0_pll_a clock.
 * @sclk_bus1_pll_a: frequency of sclk_bus1_pll_a clock.
 */
struct exynos7420_clk_topc_priv {
	struct exynos7420_clk_cmu_topc *topc;
	unsigned long fin_freq;
	unsigned long sclk_bus0_pll_a;
	unsigned long sclk_bus1_pll_a;
};

/**
 * struct exynos7420_clk_top0_priv - private data for CMU top0 clock driver.
 *
 * @top0: base address of the memory mapped CMU TOP0 controller.
 * @mout_top0_bus0_pll_half: frequency of mout_top0_bus0_pll_half clock
 * @sclk_uart2: frequency of sclk_uart2 clock.
 */
struct exynos7420_clk_top0_priv {
	struct exynos7420_clk_cmu_top0 *top0;
	unsigned long mout_top0_bus0_pll_half;
	unsigned long sclk_uart2;
};

static unsigned long pll145x_get_rate(unsigned int *con1,
				      unsigned long fin_freq)
{
	unsigned long pll_con1 = readl(con1);
	unsigned long mdiv, sdiv, pdiv;
	u64 fvco = fin_freq;

	mdiv = (pll_con1 >> PLL145X_MDIV_SHIFT) & PLL145X_MDIV_MASK;
	pdiv = (pll_con1 >> PLL145X_PDIV_SHIFT) & PLL145X_PDIV_MASK;
	sdiv = (pll_con1 >> PLL145X_SDIV_SHIFT) & PLL145X_SDIV_MASK;

	fvco *= mdiv;
	do_div(fvco, (pdiv << sdiv));
	return (unsigned long)fvco;
}

static ulong exynos7420_topc_get_rate(struct clk *clk)
{
	struct exynos7420_clk_topc_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case DOUT_SCLK_BUS0_PLL:
	case SCLK_BUS0_PLL_A:
	case SCLK_BUS0_PLL_B:
		return priv->sclk_bus0_pll_a;
	case DOUT_SCLK_BUS1_PLL:
	case SCLK_BUS1_PLL_A:
	case SCLK_BUS1_PLL_B:
		return priv->sclk_bus1_pll_a;
	default:
		return 0;
	}
}

static struct clk_ops exynos7420_clk_topc_ops = {
	.get_rate	= exynos7420_topc_get_rate,
};

static int exynos7420_clk_topc_probe(struct udevice *dev)
{
	struct exynos7420_clk_topc_priv *priv = dev_get_priv(dev);
	struct exynos7420_clk_cmu_topc *topc;
	struct clk in_clk;
	unsigned long rate;
	fdt_addr_t base;
	int ret;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	topc = (struct exynos7420_clk_cmu_topc *)base;
	priv->topc = topc;

	ret = clk_get_by_index(dev, 0, &in_clk);
	if (ret >= 0)
		priv->fin_freq = clk_get_rate(&in_clk);

	rate = pll145x_get_rate(&topc->bus0_pll_con[0], priv->fin_freq);
	if (readl(&topc->mux_sel[1]) & (1 << 16))
		rate >>= 1;
	rate /= DIVIDER(&topc->div[3], 0, 0xf);
	priv->sclk_bus0_pll_a = rate;

	rate = pll145x_get_rate(&topc->bus1_pll_con[0], priv->fin_freq) /
			DIVIDER(&topc->div[3], 8, 0xf);
	priv->sclk_bus1_pll_a = rate;

	return 0;
}

static ulong exynos7420_top0_get_rate(struct clk *clk)
{
	struct exynos7420_clk_top0_priv *priv = dev_get_priv(clk->dev);
	struct exynos7420_clk_cmu_top0 *top0 = priv->top0;

	switch (clk->id) {
	case CLK_SCLK_UART2:
		return priv->mout_top0_bus0_pll_half /
			DIVIDER(&top0->div_peric[3], 8, 0xf);
	default:
		return 0;
	}
}

static struct clk_ops exynos7420_clk_top0_ops = {
	.get_rate	= exynos7420_top0_get_rate,
};

static int exynos7420_clk_top0_probe(struct udevice *dev)
{
	struct exynos7420_clk_top0_priv *priv;
	struct exynos7420_clk_cmu_top0 *top0;
	struct clk in_clk;
	fdt_addr_t base;
	int ret;

	priv = dev_get_priv(dev);
	if (!priv)
		return -EINVAL;

	base = dev_read_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	top0 = (struct exynos7420_clk_cmu_top0 *)base;
	priv->top0 = top0;

	ret = clk_get_by_index(dev, 1, &in_clk);
	if (ret >= 0) {
		priv->mout_top0_bus0_pll_half =
			clk_get_rate(&in_clk);
		if (readl(&top0->mux_sel[1]) & (1 << 16))
			priv->mout_top0_bus0_pll_half >>= 1;
	}

	return 0;
}

static ulong exynos7420_peric1_get_rate(struct clk *clk)
{
	struct clk in_clk;
	int ret;
	unsigned long freq = 0;

	switch (clk->id) {
	case SCLK_UART2:
		ret = clk_get_by_index(clk->dev, 3, &in_clk);
		if (ret < 0)
			return 0;
		freq = clk_get_rate(&in_clk);
		break;
	}

	return freq;
}

static struct clk_ops exynos7420_clk_peric1_ops = {
	.get_rate	= exynos7420_peric1_get_rate,
};

static const struct udevice_id exynos7420_clk_topc_compat[] = {
	{ .compatible = "samsung,exynos7-clock-topc" },
	{ }
};

U_BOOT_DRIVER(exynos7420_clk_topc) = {
	.name = "exynos7420-clock-topc",
	.id = UCLASS_CLK,
	.of_match = exynos7420_clk_topc_compat,
	.probe = exynos7420_clk_topc_probe,
	.priv_auto	= sizeof(struct exynos7420_clk_topc_priv),
	.ops = &exynos7420_clk_topc_ops,
};

static const struct udevice_id exynos7420_clk_top0_compat[] = {
	{ .compatible = "samsung,exynos7-clock-top0" },
	{ }
};

U_BOOT_DRIVER(exynos7420_clk_top0) = {
	.name = "exynos7420-clock-top0",
	.id = UCLASS_CLK,
	.of_match = exynos7420_clk_top0_compat,
	.probe = exynos7420_clk_top0_probe,
	.priv_auto	= sizeof(struct exynos7420_clk_top0_priv),
	.ops = &exynos7420_clk_top0_ops,
};

static const struct udevice_id exynos7420_clk_peric1_compat[] = {
	{ .compatible = "samsung,exynos7-clock-peric1" },
	{ }
};

U_BOOT_DRIVER(exynos7420_clk_peric1) = {
	.name = "exynos7420-clock-peric1",
	.id = UCLASS_CLK,
	.of_match = exynos7420_clk_peric1_compat,
	.ops = &exynos7420_clk_peric1_ops,
};

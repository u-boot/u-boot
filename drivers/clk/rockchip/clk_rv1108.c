/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <bitfield.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rv1108.h>
#include <asm/arch/hardware.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rv1108-cru.h>

DECLARE_GLOBAL_DATA_PTR;

enum {
	VCO_MAX_HZ	= 2400U * 1000000,
	VCO_MIN_HZ	= 600 * 1000000,
	OUTPUT_MAX_HZ	= 2400U * 1000000,
	OUTPUT_MIN_HZ	= 24 * 1000000,
};

#define DIV_TO_RATE(input_rate, div)	((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _refdiv, _postdiv1, _postdiv2) {\
	.refdiv = _refdiv,\
	.fbdiv = (u32)((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ),\
	.postdiv1 = _postdiv1, .postdiv2 = _postdiv2};\
	_Static_assert(((u64)hz * _refdiv * _postdiv1 * _postdiv2 / OSC_HZ) *\
			 OSC_HZ / (_refdiv * _postdiv1 * _postdiv2) == hz,\
			 #hz "Hz cannot be hit with PLL "\
			 "divisors on line " __stringify(__LINE__));

/* use integer mode */
static inline int rv1108_pll_id(enum rk_clk_id clk_id)
{
	int id = 0;

	switch (clk_id) {
	case CLK_ARM:
	case CLK_DDR:
		id = clk_id - 1;
		break;
	case CLK_GENERAL:
		id = 2;
		break;
	default:
		printf("invalid pll id:%d\n", clk_id);
		id = -1;
		break;
	}

	return id;
}

static uint32_t rkclk_pll_get_rate(struct rv1108_cru *cru,
				   enum rk_clk_id clk_id)
{
	uint32_t refdiv, fbdiv, postdiv1, postdiv2;
	uint32_t con0, con1, con3;
	int pll_id = rv1108_pll_id(clk_id);
	struct rv1108_pll *pll = &cru->pll[pll_id];
	uint32_t freq;

	con3 = readl(&pll->con3);

	if (con3 & WORK_MODE_MASK) {
		con0 = readl(&pll->con0);
		con1 = readl(&pll->con1);
		fbdiv = (con0 >> FBDIV_SHIFT) & FBDIV_MASK;
		postdiv1 = (con1 & POSTDIV1_MASK) >> POSTDIV1_SHIFT;
		postdiv2 = (con1 & POSTDIV2_MASK) >> POSTDIV2_SHIFT;
		refdiv = (con1 & REFDIV_MASK) >> REFDIV_SHIFT;
		freq = (24 * fbdiv / (refdiv * postdiv1 * postdiv2)) * 1000000;
	} else {
		freq = OSC_HZ;
	}

	return freq;
}

static int rv1108_mac_set_clk(struct rv1108_cru *cru, ulong rate)
{
	uint32_t con = readl(&cru->clksel_con[24]);
	ulong pll_rate;
	uint8_t div;

	if ((con >> MAC_PLL_SEL_SHIFT) & MAC_PLL_SEL_GPLL)
		pll_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);
	else
		pll_rate = rkclk_pll_get_rate(cru, CLK_ARM);

	/*default set 50MHZ for gmac*/
	if (!rate)
		rate = 50000000;

	div = DIV_ROUND_UP(pll_rate, rate) - 1;
	if (div <= 0x1f)
		rk_clrsetreg(&cru->clksel_con[24], MAC_CLK_DIV_MASK,
			     div << MAC_CLK_DIV_SHIFT);
	else
		debug("Unsupported div for gmac:%d\n", div);

	return DIV_TO_RATE(pll_rate, div);
}

static int rv1108_sfc_set_clk(struct rv1108_cru *cru, uint rate)
{
	u32 con = readl(&cru->clksel_con[27]);
	u32 pll_rate;
	u32 div;

	if ((con >> SFC_PLL_SEL_SHIFT) && SFC_PLL_SEL_GPLL)
		pll_rate = rkclk_pll_get_rate(cru, CLK_GENERAL);
	else
		pll_rate = rkclk_pll_get_rate(cru, CLK_DDR);

	div = DIV_ROUND_UP(pll_rate, rate) - 1;
	if (div <= 0x3f)
		rk_clrsetreg(&cru->clksel_con[27], SFC_CLK_DIV_MASK,
			     div << SFC_CLK_DIV_SHIFT);
	else
		debug("Unsupported sfc clk rate:%d\n", rate);

	return DIV_TO_RATE(pll_rate, div);
}

static ulong rv1108_saradc_get_clk(struct rv1108_cru *cru)
{
	u32 div, val;

	val = readl(&cru->clksel_con[22]);
	div = bitfield_extract(val, CLK_SARADC_DIV_CON_SHIFT,
			       CLK_SARADC_DIV_CON_WIDTH);

	return DIV_TO_RATE(OSC_HZ, div);
}

static ulong rv1108_saradc_set_clk(struct rv1108_cru *cru, uint hz)
{
	int src_clk_div;

	src_clk_div = DIV_ROUND_UP(OSC_HZ, hz) - 1;
	assert(src_clk_div < 128);

	rk_clrsetreg(&cru->clksel_con[22],
		     CLK_SARADC_DIV_CON_MASK,
		     src_clk_div << CLK_SARADC_DIV_CON_SHIFT);

	return rv1108_saradc_get_clk(cru);
}

static ulong rv1108_clk_get_rate(struct clk *clk)
{
	struct rv1108_clk_priv *priv = dev_get_priv(clk->dev);

	switch (clk->id) {
	case 0 ... 63:
		return rkclk_pll_get_rate(priv->cru, clk->id);
	case SCLK_SARADC:
		return rv1108_saradc_get_clk(priv->cru);
	default:
		return -ENOENT;
	}
}

static ulong rv1108_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rv1108_clk_priv *priv = dev_get_priv(clk->dev);
	ulong new_rate;

	switch (clk->id) {
	case SCLK_MAC:
		new_rate = rv1108_mac_set_clk(priv->cru, rate);
		break;
	case SCLK_SFC:
		new_rate = rv1108_sfc_set_clk(priv->cru, rate);
		break;
	case SCLK_SARADC:
		new_rate = rv1108_saradc_set_clk(priv->cru, rate);
		break;
	default:
		return -ENOENT;
	}

	return new_rate;
}

static const struct clk_ops rv1108_clk_ops = {
	.get_rate	= rv1108_clk_get_rate,
	.set_rate	= rv1108_clk_set_rate,
};

static void rkclk_init(struct rv1108_cru *cru)
{
	unsigned int apll = rkclk_pll_get_rate(cru, CLK_ARM);
	unsigned int dpll = rkclk_pll_get_rate(cru, CLK_DDR);
	unsigned int gpll = rkclk_pll_get_rate(cru, CLK_GENERAL);

	rk_clrsetreg(&cru->clksel_con[0], CORE_CLK_DIV_MASK,
		     0 << MAC_CLK_DIV_SHIFT);

	printf("APLL: %d DPLL:%d GPLL:%d\n", apll, dpll, gpll);
}

static int rv1108_clk_probe(struct udevice *dev)
{
	struct rv1108_clk_priv *priv = dev_get_priv(dev);

	priv->cru = (struct rv1108_cru *)devfdt_get_addr(dev);

	rkclk_init(priv->cru);

	return 0;
}

static int rv1108_clk_bind(struct udevice *dev)
{
	int ret;
	struct udevice *sys_child;
	struct sysreset_reg *priv;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(dev, "rockchip_sysreset", "sysreset",
				 &sys_child);
	if (ret) {
		debug("Warning: No sysreset driver: ret=%d\n", ret);
	} else {
		priv = malloc(sizeof(struct sysreset_reg));
		priv->glb_srst_fst_value = offsetof(struct rv1108_cru,
						    glb_srst_fst_val);
		priv->glb_srst_snd_value = offsetof(struct rv1108_cru,
						    glb_srst_snd_val);
		sys_child->priv = priv;
	}

#if CONFIG_IS_ENABLED(CONFIG_RESET_ROCKCHIP)
	ret = offsetof(struct rk3368_cru, softrst_con[0]);
	ret = rockchip_reset_bind(dev, ret, 13);
	if (ret)
		debug("Warning: software reset driver bind faile\n");
#endif

	return 0;
}

static const struct udevice_id rv1108_clk_ids[] = {
	{ .compatible = "rockchip,rv1108-cru" },
	{ }
};

U_BOOT_DRIVER(clk_rv1108) = {
	.name		= "clk_rv1108",
	.id		= UCLASS_CLK,
	.of_match	= rv1108_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rv1108_clk_priv),
	.ops		= &rv1108_clk_ops,
	.bind		= rv1108_clk_bind,
	.probe		= rv1108_clk_probe,
};

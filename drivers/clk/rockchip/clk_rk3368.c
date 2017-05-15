/*
 * (C) Copyright 2017 Rockchip Electronics Co., Ltd
 * Author: Andy Yan <andy.yan@rock-chips.com>
 * SPDX-License-Identifier:	GPL-2.0
 */

#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <errno.h>
#include <syscon.h>
#include <asm/arch/clock.h>
#include <asm/arch/cru_rk3368.h>
#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/lists.h>
#include <dt-bindings/clock/rk3368-cru.h>

DECLARE_GLOBAL_DATA_PTR;

struct pll_div {
	u32 nr;
	u32 nf;
	u32 no;
};

#define OSC_HZ		(24 * 1000 * 1000)
#define APLL_L_HZ	(800 * 1000 * 1000)
#define APLL_B_HZ	(816 * 1000 * 1000)
#define GPLL_HZ		(576 * 1000 * 1000)
#define CPLL_HZ		(400 * 1000 * 1000)

#define RATE_TO_DIV(input_rate, output_rate) \
		((input_rate) / (output_rate) - 1);

#define DIV_TO_RATE(input_rate, div)    ((input_rate) / ((div) + 1))

#define PLL_DIVISORS(hz, _nr, _no) { \
	.nr = _nr, .nf = (u32)((u64)hz * _nr * _no / OSC_HZ), .no = _no}; \
	_Static_assert(((u64)hz * _nr * _no / OSC_HZ) * OSC_HZ /\
		       (_nr * _no) == hz, #hz "Hz cannot be hit with PLL " \
		       "divisors on line " __stringify(__LINE__));

static const struct pll_div apll_l_init_cfg = PLL_DIVISORS(APLL_L_HZ, 12, 2);
static const struct pll_div apll_b_init_cfg = PLL_DIVISORS(APLL_B_HZ, 1, 2);
static const struct pll_div gpll_init_cfg = PLL_DIVISORS(GPLL_HZ, 1, 2);
static const struct pll_div cpll_init_cfg = PLL_DIVISORS(CPLL_HZ, 1, 6);

/* Get pll rate by id */
static uint32_t rkclk_pll_get_rate(struct rk3368_cru *cru,
				   enum rk3368_pll_id pll_id)
{
	uint32_t nr, no, nf;
	uint32_t con;
	struct rk3368_pll *pll = &cru->pll[pll_id];

	con = readl(&pll->con3);

	switch ((con & PLL_MODE_MASK) >> PLL_MODE_SHIFT) {
	case PLL_MODE_SLOW:
		return OSC_HZ;
	case PLL_MODE_NORMAL:
		con = readl(&pll->con0);
		no = ((con & PLL_OD_MASK) >> PLL_OD_SHIFT) + 1;
		nr = ((con & PLL_NR_MASK) >> PLL_NR_SHIFT) + 1;
		con = readl(&pll->con1);
		nf = ((con & PLL_NF_MASK) >> PLL_NF_SHIFT) + 1;

		return (24 * nf / (nr * no)) * 1000000;
	case PLL_MODE_DEEP_SLOW:
	default:
		return 32768;
	}
}

static int rkclk_set_pll(struct rk3368_cru *cru, enum rk3368_pll_id pll_id,
			 const struct pll_div *div, bool has_bwadj)
{
	struct rk3368_pll *pll = &cru->pll[pll_id];
	/* All PLLs have same VCO and output frequency range restrictions*/
	uint vco_hz = OSC_HZ / 1000 * div->nf / div->nr * 1000;
	uint output_hz = vco_hz / div->no;

	debug("PLL at %p: nf=%d, nr=%d, no=%d, vco=%u Hz, output=%u Hz\n",
	      pll, div->nf, div->nr, div->no, vco_hz, output_hz);

	/* enter slow mode and reset pll */
	rk_clrsetreg(&pll->con3, PLL_MODE_MASK | PLL_RESET_MASK,
		     PLL_RESET << PLL_RESET_SHIFT);

	rk_clrsetreg(&pll->con0, PLL_NR_MASK | PLL_OD_MASK,
		     ((div->nr - 1) << PLL_NR_SHIFT) |
		     ((div->no - 1) << PLL_OD_SHIFT));
	writel((div->nf - 1) << PLL_NF_SHIFT, &pll->con1);
	udelay(10);

	/* return from reset */
	rk_clrreg(&pll->con3, PLL_RESET_MASK);

	/* waiting for pll lock */
	while (!(readl(&pll->con1) & PLL_LOCK_STA))
		udelay(1);

	rk_clrsetreg(&pll->con3, PLL_MODE_MASK,
		     PLL_MODE_NORMAL << PLL_MODE_SHIFT);

	return 0;
}

static void rkclk_init(struct rk3368_cru *cru)
{
	u32 apllb, aplll, dpll, cpll, gpll;

	rkclk_set_pll(cru, APLLB, &apll_b_init_cfg, false);
	rkclk_set_pll(cru, APLLL, &apll_l_init_cfg, false);
	rkclk_set_pll(cru, GPLL, &gpll_init_cfg, false);
	rkclk_set_pll(cru, CPLL, &cpll_init_cfg, false);

	apllb = rkclk_pll_get_rate(cru, APLLB);
	aplll = rkclk_pll_get_rate(cru, APLLL);
	dpll = rkclk_pll_get_rate(cru, DPLL);
	cpll = rkclk_pll_get_rate(cru, CPLL);
	gpll = rkclk_pll_get_rate(cru, GPLL);

	debug("%s apllb(%d) apll(%d) dpll(%d) cpll(%d) gpll(%d)\n",
	       __func__, apllb, aplll, dpll, cpll, gpll);
}

static ulong rk3368_mmc_get_clk(struct rk3368_cru *cru, uint clk_id)
{
	u32 div, con, con_id, rate;
	u32 pll_rate;

	switch (clk_id) {
	case SCLK_SDMMC:
		con_id = 50;
		break;
	case SCLK_EMMC:
		con_id = 51;
		break;
	case SCLK_SDIO0:
		con_id = 48;
		break;
	default:
		return -EINVAL;
	}

	con = readl(&cru->clksel_con[con_id]);
	switch ((con & MMC_PLL_SEL_MASK) >> MMC_PLL_SEL_SHIFT) {
	case MMC_PLL_SEL_GPLL:
		pll_rate = rkclk_pll_get_rate(cru, GPLL);
		break;
	case MMC_PLL_SEL_24M:
		pll_rate = OSC_HZ;
		break;
	case MMC_PLL_SEL_CPLL:
	case MMC_PLL_SEL_USBPHY_480M:
	default:
		return -EINVAL;
	}
	div = (con & MMC_CLK_DIV_MASK) >> MMC_CLK_DIV_SHIFT;
	rate = DIV_TO_RATE(pll_rate, div);

	return rate >> 1;
}

static ulong rk3368_mmc_set_clk(struct rk3368_cru *cru,
				ulong clk_id, ulong rate)
{
	u32 div;
	u32 con_id;
	u32 gpll_rate = rkclk_pll_get_rate(cru, GPLL);

	div = RATE_TO_DIV(gpll_rate, rate << 1);

	switch (clk_id) {
	case SCLK_SDMMC:
		con_id = 50;
		break;
	case SCLK_EMMC:
		con_id = 51;
		break;
	case SCLK_SDIO0:
		con_id = 48;
		break;
	default:
		return -EINVAL;
	}

	if (div > 0x3f) {
		div = RATE_TO_DIV(OSC_HZ, rate);
		rk_clrsetreg(&cru->clksel_con[con_id],
			     MMC_PLL_SEL_MASK | MMC_CLK_DIV_MASK,
			     (MMC_PLL_SEL_24M << MMC_PLL_SEL_SHIFT) |
			     (div << MMC_CLK_DIV_SHIFT));
	} else {
		rk_clrsetreg(&cru->clksel_con[con_id],
			     MMC_PLL_SEL_MASK | MMC_CLK_DIV_MASK,
			     (MMC_PLL_SEL_GPLL << MMC_PLL_SEL_SHIFT) |
			     div << MMC_CLK_DIV_SHIFT);
	}

	return rk3368_mmc_get_clk(cru, clk_id);
}

static ulong rk3368_clk_get_rate(struct clk *clk)
{
	struct rk3368_clk_priv *priv = dev_get_priv(clk->dev);
	ulong rate = 0;

	debug("%s id:%ld\n", __func__, clk->id);
	switch (clk->id) {
	case HCLK_SDMMC:
	case HCLK_EMMC:
		rate = rk3368_mmc_get_clk(priv->cru, clk->id);
		break;
	default:
		return -ENOENT;
	}

	return rate;
}

static ulong rk3368_clk_set_rate(struct clk *clk, ulong rate)
{
	struct rk3368_clk_priv *priv = dev_get_priv(clk->dev);
	ulong ret = 0;

	debug("%s id:%ld rate:%ld\n", __func__, clk->id, rate);
	switch (clk->id) {
	case SCLK_SDMMC:
	case SCLK_EMMC:
		ret = rk3368_mmc_set_clk(priv->cru, clk->id, rate);
		break;
	default:
		return -ENOENT;
	}

	return ret;
}

static struct clk_ops rk3368_clk_ops = {
	.get_rate = rk3368_clk_get_rate,
	.set_rate = rk3368_clk_set_rate,
};

static int rk3368_clk_probe(struct udevice *dev)
{
	struct rk3368_clk_priv *priv = dev_get_priv(dev);

	rkclk_init(priv->cru);

	return 0;
}

static int rk3368_clk_ofdata_to_platdata(struct udevice *dev)
{
	struct rk3368_clk_priv *priv = dev_get_priv(dev);

	priv->cru = (struct rk3368_cru *)devfdt_get_addr(dev);

	return 0;
}

static int rk3368_clk_bind(struct udevice *dev)
{
	int ret;

	/* The reset driver does not have a device node, so bind it here */
	ret = device_bind_driver(gd->dm_root, "rk3368_sysreset", "reset", &dev);
	if (ret)
		error("bind RK3368 reset driver failed: ret=%d\n", ret);

	return ret;
}

static const struct udevice_id rk3368_clk_ids[] = {
	{ .compatible = "rockchip,rk3368-cru" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3368_cru) = {
	.name		= "rockchip_rk3368_cru",
	.id		= UCLASS_CLK,
	.of_match	= rk3368_clk_ids,
	.priv_auto_alloc_size = sizeof(struct rk3368_cru),
	.ofdata_to_platdata = rk3368_clk_ofdata_to_platdata,
	.ops		= &rk3368_clk_ops,
	.bind		= rk3368_clk_bind,
	.probe		= rk3368_clk_probe,
};

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 */

#include <clk-uclass.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>

#define TEGRA_CAR_CLK_PLL	BIT(0)
#define TEGRA_CAR_CLK_PERIPH	BIT(1)

static int tegra_car_clk_request(struct clk *clk)
{
	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	/*
	 * Note that the first PERIPH_ID_COUNT clock IDs (where the value
	 * varies per SoC) are the peripheral clocks, which use a numbering
	 * scheme that matches HW registers 1:1. There are other clock IDs
	 * beyond this that are assigned arbitrarily by the Tegra CAR DT
	 * binding.
	 */
	if (clk->id < PERIPH_ID_COUNT) {
		clk->data |= TEGRA_CAR_CLK_PERIPH;
		return 0;
	}

	/* If check for periph failed, then check for PLL clock id */
	int id = clk_id_to_pll_id(clk->id);

	if (clock_id_is_pll(id)) {
		clk->id = id;
		clk->data |= TEGRA_CAR_CLK_PLL;
		return 0;
	}

	return -EINVAL;
}

static ulong tegra_car_clk_get_rate(struct clk *clk)
{
	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	if (clk->data & TEGRA_CAR_CLK_PLL)
		return clock_get_rate(clk->id);

	if (clk->data & TEGRA_CAR_CLK_PERIPH) {
		enum clock_id parent;

		parent = clock_get_periph_parent(clk->id);
		return clock_get_periph_rate(clk->id, parent);
	}

	return -1U;
}

static ulong tegra_car_clk_set_rate(struct clk *clk, ulong rate)
{
	enum clock_id parent;

	debug("%s(clk=%p, rate=%lu) (dev=%p, id=%lu)\n", __func__, clk, rate,
	      clk->dev, clk->id);

	if (clk->data & TEGRA_CAR_CLK_PLL)
		return 0;

	parent = clock_get_periph_parent(clk->id);
	return clock_adjust_periph_pll_div(clk->id, parent, rate, NULL);
}

static int tegra_car_clk_enable(struct clk *clk)
{
	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	if (clk->data & TEGRA_CAR_CLK_PLL)
		return 0;

	clock_enable(clk->id);

	return 0;
}

static int tegra_car_clk_disable(struct clk *clk)
{
	debug("%s(clk=%p) (dev=%p, id=%lu)\n", __func__, clk, clk->dev,
	      clk->id);

	if (clk->data & TEGRA_CAR_CLK_PLL)
		return 0;

	clock_disable(clk->id);

	return 0;
}

static struct clk_ops tegra_car_clk_ops = {
	.request = tegra_car_clk_request,
	.get_rate = tegra_car_clk_get_rate,
	.set_rate = tegra_car_clk_set_rate,
	.enable = tegra_car_clk_enable,
	.disable = tegra_car_clk_disable,
};

static int tegra_car_clk_probe(struct udevice *dev)
{
	debug("%s(dev=%p)\n", __func__, dev);

	clock_init();
	clock_verify();

	return 0;
}

U_BOOT_DRIVER(tegra_car_clk) = {
	.name = "tegra_car_clk",
	.id = UCLASS_CLK,
	.probe = tegra_car_clk_probe,
	.ops = &tegra_car_clk_ops,
};

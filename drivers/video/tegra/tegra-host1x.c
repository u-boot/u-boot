// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#include <dm.h>
#include <clk.h>
#include <log.h>
#include <reset.h>
#include <linux/delay.h>

#include <asm/arch/clock.h>
#include <asm/arch-tegra/clk_rst.h>

struct tegra_host1x_info {
	u32 clk_parent;
	u32 rate;
};

static int tegra_host1x_probe(struct udevice *dev)
{
	struct clk *clk;
	struct reset_ctl reset_ctl;
	const struct tegra_host1x_info *info;
	int ret;

	clk = devm_clk_get(dev, NULL);
	if (IS_ERR(clk)) {
		log_debug("%s: cannot get HOST1X clock: %ld\n",
			  __func__, PTR_ERR(clk));
		return PTR_ERR(clk);
	}

	ret = reset_get_by_name(dev, "host1x", &reset_ctl);
	if (ret) {
		log_debug("%s: cannot get HOST1X reset: %d\n",
			  __func__, ret);
		return ret;
	}

	info = (struct tegra_host1x_info *)dev_get_driver_data(dev);

	reset_assert(&reset_ctl);
	clock_start_periph_pll(clk->id, info->clk_parent, info->rate);

	mdelay(2);
	reset_deassert(&reset_ctl);

	return 0;
}

static const struct tegra_host1x_info tegra20_host1x_info = {
	.clk_parent = CLOCK_ID_CGENERAL,
	.rate = 150000000, /* 150 MHz */
};

static const struct tegra_host1x_info tegra114_host1x_info = {
	.clk_parent = CLOCK_ID_PERIPH,
	.rate = 136000000, /* 136 MHz */
};

static const struct udevice_id tegra_host1x_ids[] = {
	{
		.compatible = "nvidia,tegra20-host1x",
		.data = (ulong)&tegra20_host1x_info
	}, {
		.compatible = "nvidia,tegra30-host1x",
		.data = (ulong)&tegra20_host1x_info
	}, {
		.compatible = "nvidia,tegra114-host1x",
		.data = (ulong)&tegra114_host1x_info
	}, {
		.compatible = "nvidia,tegra124-host1x",
		.data = (ulong)&tegra114_host1x_info
	}, {
		/* sentinel */
	}
};

U_BOOT_DRIVER(tegra_host1x) = {
	.name		= "tegra_host1x",
	.id		= UCLASS_SIMPLE_BUS,
	.of_match	= tegra_host1x_ids,
	.probe		= tegra_host1x_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};

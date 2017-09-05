/*
 * Copyright (C) 2017 Theobroma Systems Design und Consulting GmbH
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <mapmem.h>
#include <asm/arch/timer.h>
#include <dt-structs.h>
#include <timer.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#if CONFIG_IS_ENABLED(OF_PLATDATA)
struct rockchip_timer_plat {
	struct dtd_rockchip_rk3368_timer dtd;
};
#endif

/* Driver private data. Contains timer id. Could be either 0 or 1. */
struct rockchip_timer_priv {
	struct rk_timer *timer;
};

static int rockchip_timer_get_count(struct udevice *dev, u64 *count)
{
	struct rockchip_timer_priv *priv = dev_get_priv(dev);
	uint64_t timebase_h, timebase_l;
	uint64_t cntr;

	timebase_l = readl(&priv->timer->timer_curr_value0);
	timebase_h = readl(&priv->timer->timer_curr_value1);

	/* timers are down-counting */
	cntr = timebase_h << 32 | timebase_l;
	*count = ~0ull - cntr;
	return 0;
}

static int rockchip_clk_ofdata_to_platdata(struct udevice *dev)
{
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	struct rockchip_timer_priv *priv = dev_get_priv(dev);

	priv->timer = (struct rk_timer *)devfdt_get_addr(dev);
#endif

	return 0;
}

static int rockchip_timer_start(struct udevice *dev)
{
	struct rockchip_timer_priv *priv = dev_get_priv(dev);
	const uint64_t reload_val = ~0uLL;
	const uint32_t reload_val_l = reload_val & 0xffffffff;
	const uint32_t reload_val_h = reload_val >> 32;

	/* disable timer and reset all control */
	writel(0, &priv->timer->timer_ctrl_reg);
	/* write reload value */
	writel(reload_val_l, &priv->timer->timer_load_count0);
	writel(reload_val_h, &priv->timer->timer_load_count1);
	/* enable timer */
	writel(1, &priv->timer->timer_ctrl_reg);

	return 0;
}

static int rockchip_timer_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rockchip_timer_priv *priv = dev_get_priv(dev);
	struct rockchip_timer_plat *plat = dev_get_platdata(dev);

	priv->timer = map_sysmem(plat->dtd.reg[1], plat->dtd.reg[3]);
	uc_priv->clock_rate = plat->dtd.clock_frequency;
#endif

	return rockchip_timer_start(dev);
}

static const struct timer_ops rockchip_timer_ops = {
	.get_count = rockchip_timer_get_count,
};

static const struct udevice_id rockchip_timer_ids[] = {
	{ .compatible = "rockchip,rk3368-timer" },
	{}
};

U_BOOT_DRIVER(rockchip_rk3368_timer) = {
	.name	= "rockchip_rk3368_timer",
	.id	= UCLASS_TIMER,
	.of_match = rockchip_timer_ids,
	.probe = rockchip_timer_probe,
	.ops	= &rockchip_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto_alloc_size = sizeof(struct rockchip_timer_priv),
#if CONFIG_IS_ENABLED(OF_PLATDATA)
	.platdata_auto_alloc_size = sizeof(struct rockchip_timer_plat),
#endif
	.ofdata_to_platdata = rockchip_clk_ofdata_to_platdata,
};

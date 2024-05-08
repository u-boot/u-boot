// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * (C) Copyright 2022 - Analog Devices, Inc.
 *
 * Written and/or maintained by Timesys Corporation
 *
 * Converted to driver model by Nathan Barrett-Morrison
 *
 * Author: Greg Malysa <greg.malysa@timesys.com>
 * Additional Contact: Nathan Barrett-Morrison <nathan.morrison@timesys.com>
 *
 * dm timer implementation for ADI ADSP-SC5xx SoCs
 *
 */

#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/compiler_types.h>

/*
 * Timer Configuration Register Bits
 */
#define TIMER_OUT_DIS       0x0800
#define TIMER_PULSE_HI      0x0080
#define TIMER_MODE_PWM_CONT 0x000c

#define __BFP(m) u16 m; u16 __pad_##m

struct gptimer3 {
	__BFP(config);
	u32 counter;
	u32 period;
	u32 width;
	u32 delay;
};

struct gptimer3_group_regs {
	__BFP(run);
	__BFP(enable);
	__BFP(disable);
	__BFP(stop_cfg);
	__BFP(stop_cfg_set);
	__BFP(stop_cfg_clr);
	__BFP(data_imsk);
	__BFP(stat_imsk);
	__BFP(tr_msk);
	__BFP(tr_ie);
	__BFP(data_ilat);
	__BFP(stat_ilat);
	__BFP(err_status);
	__BFP(bcast_per);
	__BFP(bcast_wid);
	__BFP(bcast_dly);
};

#define MAX_TIM_LOAD	0xFFFFFFFF

struct adi_gptimer_priv {
	struct gptimer3_group_regs __iomem *timer_group;
	struct gptimer3 __iomem *timer_base;
	u32 prev;
	u64 upper;
};

static u64 adi_gptimer_get_count(struct udevice *udev)
{
	struct adi_gptimer_priv *priv = dev_get_priv(udev);

	u32 now = readl(&priv->timer_base->counter);

	if (now < priv->prev)
		priv->upper += (1ull << 32);

	priv->prev = now;

	return (priv->upper + (u64)now);
}

static const struct timer_ops adi_gptimer_ops = {
	.get_count = adi_gptimer_get_count,
};

static int adi_gptimer_probe(struct udevice *udev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(udev);
	struct adi_gptimer_priv *priv = dev_get_priv(udev);
	struct clk clk;
	u16 imask;
	int ret;

	priv->timer_group = dev_remap_addr_index(udev, 0);
	priv->timer_base = dev_remap_addr_index(udev, 1);
	priv->upper = 0;
	priv->prev = 0;

	if (!priv->timer_group || !priv->timer_base) {
		dev_err(udev, "Missing timer_group or timer_base reg entries\n");
		return -ENODEV;
	}

	ret = clk_get_by_index(udev, 0, &clk);
	if (ret < 0) {
		dev_err(udev, "Missing clock reference for timer\n");
		return ret;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(udev, "Failed to enable clock\n");
		return ret;
	}

	uc_priv->clock_rate = clk_get_rate(&clk);

	/* Enable timer */
	writew(TIMER_OUT_DIS | TIMER_MODE_PWM_CONT | TIMER_PULSE_HI,
	       &priv->timer_base->config);
	writel(MAX_TIM_LOAD, &priv->timer_base->period);
	writel(MAX_TIM_LOAD - 1, &priv->timer_base->width);

	/* We only use timer 0 in uboot */
	imask = readw(&priv->timer_group->data_imsk);
	imask &= ~(1 << 0);
	writew(imask, &priv->timer_group->data_imsk);
	writew((1 << 0), &priv->timer_group->enable);

	return 0;
}

static const struct udevice_id adi_gptimer_ids[] = {
	{ .compatible = "adi,sc5xx-gptimer" },
	{ },
};

U_BOOT_DRIVER(adi_gptimer) = {
	.name = "adi_gptimer",
	.id = UCLASS_TIMER,
	.of_match = adi_gptimer_ids,
	.priv_auto = sizeof(struct adi_gptimer_priv),
	.probe = adi_gptimer_probe,
	.ops = &adi_gptimer_ops,
};

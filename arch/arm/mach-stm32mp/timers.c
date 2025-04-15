// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2025, STMicroelectronics - All Rights Reserved
 * Author: Cheick Traore <cheick.traore@foss.st.com>
 *
 * Originally based on the Linux kernel v6.1 drivers/mfd/stm32-timers.c.
 */

#include <dm.h>
#include <asm/io.h>
#include <asm/arch/timers.h>
#include <dm/device_compat.h>

static void stm32_timers_get_arr_size(struct udevice *dev)
{
	struct stm32_timers_plat *plat = dev_get_plat(dev);
	struct stm32_timers_priv *priv = dev_get_priv(dev);
	u32 arr;

	/* Backup ARR to restore it after getting the maximum value */
	arr = readl(plat->base + TIM_ARR);

	/*
	 * Only the available bits will be written so when readback
	 * we get the maximum value of auto reload register
	 */
	writel(~0L, plat->base + TIM_ARR);
	priv->max_arr = readl(plat->base + TIM_ARR);
	writel(arr, plat->base + TIM_ARR);
}

static int stm32_timers_of_to_plat(struct udevice *dev)
{
	struct stm32_timers_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base) {
		dev_err(dev, "can't get address\n");
		return -ENOENT;
	}

	return 0;
}

static int stm32_timers_probe(struct udevice *dev)
{
	struct stm32_timers_priv *priv = dev_get_priv(dev);
	struct clk clk;
	int ret = 0;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock: ret=%d\n", ret);
		return ret;
	}

	priv->rate = clk_get_rate(&clk);

	stm32_timers_get_arr_size(dev);

	return ret;
}

static const struct udevice_id stm32_timers_ids[] = {
	{ .compatible = "st,stm32-timers" },
	{}
};

U_BOOT_DRIVER(stm32_timers) = {
	.name  = "stm32_timers",
	.id = UCLASS_NOP,
	.of_match = stm32_timers_ids,
	.of_to_plat = stm32_timers_of_to_plat,
	.plat_auto = sizeof(struct stm32_timers_plat),
	.probe = stm32_timers_probe,
	.priv_auto = sizeof(struct stm32_timers_priv),
	.bind = dm_scan_fdt_dev,
};

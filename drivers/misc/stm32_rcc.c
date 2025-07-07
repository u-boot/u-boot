// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@foss.st.com> for STMicroelectronics.
 */

#define LOG_CATEGORY UCLASS_NOP

#include <dm.h>
#include <log.h>
#include <misc.h>
#include <stm32_rcc.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>

static const struct stm32_rcc stm32_rcc_f42x = {
	.drv_name_clk = "stm32fx_rcc_clock",
	.drv_name_rst = "stm32_rcc_reset",
	.soc = STM32F42X,
};

static const struct stm32_rcc stm32_rcc_f469 = {
	.drv_name_clk = "stm32fx_rcc_clock",
	.drv_name_rst = "stm32_rcc_reset",
	.soc = STM32F469,
};

static const struct stm32_rcc stm32_rcc_f7 = {
	.drv_name_clk = "stm32fx_rcc_clock",
	.drv_name_rst = "stm32_rcc_reset",
	.soc = STM32F7,
};

static const struct stm32_rcc stm32_rcc_h7 = {
	.drv_name_clk = "stm32h7_rcc_clock",
	.drv_name_rst = "stm32_rcc_reset",
};

static const struct stm32_rcc stm32_rcc_mp15 = {
	.drv_name_clk = "stm32mp1_clk",
	.drv_name_rst = "stm32mp1_reset",
};

static const struct stm32_rcc stm32_rcc_mp13 = {
	.drv_name_clk = "stm32mp13_clk",
	.drv_name_rst = "stm32mp1_reset",
};

static const struct stm32_rcc stm32_rcc_mp25 = {
	.drv_name_clk = "stm32mp25_clk",
	.drv_name_rst = "stm32mp25_reset",
};

static int stm32_rcc_bind(struct udevice *dev)
{
	struct udevice *child;
	struct driver *drv;
	struct stm32_rcc *rcc_clk =
		(struct stm32_rcc *)dev_get_driver_data(dev);
	int ret;

	dev_dbg(dev, "RCC bind\n");
	drv = lists_driver_lookup_name(rcc_clk->drv_name_clk);
	if (!drv) {
		dev_err(dev, "Cannot find driver '%s'\n", rcc_clk->drv_name_clk);
		return -ENOENT;
	}

	ret = device_bind_with_driver_data(dev, drv, dev->name,
					   rcc_clk->soc,
					   dev_ofnode(dev), &child);

	if (ret)
		return ret;

	drv = lists_driver_lookup_name(rcc_clk->drv_name_rst);
	if (!drv) {
		dev_err(dev, "Cannot find driver stm32_rcc_reset'\n");
		return -ENOENT;
	}

	return device_bind(dev, drv, dev->name, NULL, dev_ofnode(dev), &child);
}

static const struct udevice_id stm32_rcc_ids[] = {
	{.compatible = "st,stm32f42xx-rcc", .data = (ulong)&stm32_rcc_f42x },
	{.compatible = "st,stm32f469-rcc", .data = (ulong)&stm32_rcc_f469 },
	{.compatible = "st,stm32f746-rcc", .data = (ulong)&stm32_rcc_f7 },
	{.compatible = "st,stm32h743-rcc", .data = (ulong)&stm32_rcc_h7 },
	{.compatible = "st,stm32mp1-rcc", .data = (ulong)&stm32_rcc_mp15 },
	{.compatible = "st,stm32mp1-rcc-secure", .data = (ulong)&stm32_rcc_mp15 },
	{.compatible = "st,stm32mp13-rcc", .data = (ulong)&stm32_rcc_mp13 },
	{.compatible = "st,stm32mp25-rcc", .data = (ulong)&stm32_rcc_mp25 },
	{ }
};

U_BOOT_DRIVER(stm32_rcc) = {
	.name		= "stm32-rcc",
	.id		= UCLASS_NOP,
	.of_match	= stm32_rcc_ids,
	.bind		= stm32_rcc_bind,
};

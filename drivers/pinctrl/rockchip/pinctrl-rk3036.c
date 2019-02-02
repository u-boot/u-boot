// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019 Rockchip Electronics Co., Ltd
 */

#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>

#include "pinctrl-rockchip.h"

#define RK3036_PULL_OFFSET		0x118
#define RK3036_PULL_PINS_PER_REG	16
#define RK3036_PULL_BANK_STRIDE		8

static void rk3036_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	*reg = RK3036_PULL_OFFSET;
	*reg += bank->bank_num * RK3036_PULL_BANK_STRIDE;
	*reg += (pin_num / RK3036_PULL_PINS_PER_REG) * 4;

	*bit = pin_num % RK3036_PULL_PINS_PER_REG;
};

static struct rockchip_pin_bank rk3036_pin_banks[] = {
	PIN_BANK(0, 32, "gpio0"),
	PIN_BANK(1, 32, "gpio1"),
	PIN_BANK(2, 32, "gpio2"),
};

static struct rockchip_pin_ctrl rk3036_pin_ctrl = {
		.pin_banks		= rk3036_pin_banks,
		.nr_banks		= ARRAY_SIZE(rk3036_pin_banks),
		.label			= "RK3036-GPIO",
		.type			= RK3036,
		.grf_mux_offset		= 0xa8,
		.pull_calc_reg		= rk3036_calc_pull_reg_and_bit,
};

static const struct udevice_id rk3036_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3036-pinctrl",
		.data = (ulong)&rk3036_pin_ctrl
	},
	{}
};

U_BOOT_DRIVER(pinctrl_rockchip) = {
	.name		= "rk3036-pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3036_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

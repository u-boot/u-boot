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

#define RK3368_PULL_GRF_OFFSET		0x100
#define RK3368_PULL_PMU_OFFSET		0x10

static void rk3368_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The first 32 pins of the first bank are located in PMU */
	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = RK3368_PULL_PMU_OFFSET;

		*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);
		*bit = pin_num % ROCKCHIP_PULL_PINS_PER_REG;
		*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
	} else {
		*regmap = priv->regmap_base;
		*reg = RK3368_PULL_GRF_OFFSET;

		/* correct the offset, as we're starting with the 2nd bank */
		*reg -= 0x10;
		*reg += bank->bank_num * ROCKCHIP_PULL_BANK_STRIDE;
		*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);

		*bit = (pin_num % ROCKCHIP_PULL_PINS_PER_REG);
		*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
	}
}

#define RK3368_DRV_PMU_OFFSET		0x20
#define RK3368_DRV_GRF_OFFSET		0x200

static void rk3368_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The first 32 pins of the first bank are located in PMU */
	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = RK3368_DRV_PMU_OFFSET;

		*reg += ((pin_num / ROCKCHIP_DRV_PINS_PER_REG) * 4);
		*bit = pin_num % ROCKCHIP_DRV_PINS_PER_REG;
		*bit *= ROCKCHIP_DRV_BITS_PER_PIN;
	} else {
		*regmap = priv->regmap_base;
		*reg = RK3368_DRV_GRF_OFFSET;

		/* correct the offset, as we're starting with the 2nd bank */
		*reg -= 0x10;
		*reg += bank->bank_num * ROCKCHIP_DRV_BANK_STRIDE;
		*reg += ((pin_num / ROCKCHIP_DRV_PINS_PER_REG) * 4);

		*bit = (pin_num % ROCKCHIP_DRV_PINS_PER_REG);
		*bit *= ROCKCHIP_DRV_BITS_PER_PIN;
	}
}

static struct rockchip_pin_bank rk3368_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS(0, 32, "gpio0", IOMUX_SOURCE_PMU,
					     IOMUX_SOURCE_PMU,
					     IOMUX_SOURCE_PMU,
					     IOMUX_SOURCE_PMU
			    ),
	PIN_BANK(1, 32, "gpio1"),
	PIN_BANK(2, 32, "gpio2"),
	PIN_BANK(3, 32, "gpio3"),
};

static struct rockchip_pin_ctrl rk3368_pin_ctrl = {
	.pin_banks		= rk3368_pin_banks,
	.nr_banks		= ARRAY_SIZE(rk3368_pin_banks),
	.label			= "RK3368-GPIO",
	.type			= RK3368,
	.grf_mux_offset		= 0x0,
	.pmu_mux_offset		= 0x0,
	.pull_calc_reg		= rk3368_calc_pull_reg_and_bit,
	.drv_calc_reg		= rk3368_calc_drv_reg_and_bit,
};

static const struct udevice_id rk3368_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3368-pinctrl",
		.data = (ulong)&rk3368_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3368) = {
	.name		= "rockchip_rk3368_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3368_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

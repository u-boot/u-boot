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

static struct rockchip_mux_route_data rk3288_mux_route_data[] = {
	{
		/* edphdmi_cecinoutt1 */
		.bank_num = 7,
		.pin = 16,
		.func = 2,
		.route_offset = 0x264,
		.route_val = BIT(16 + 12) | BIT(12),
	}, {
		/* edphdmi_cecinout */
		.bank_num = 7,
		.pin = 23,
		.func = 4,
		.route_offset = 0x264,
		.route_val = BIT(16 + 12),
	},
};

#define RK3288_PULL_OFFSET		0x140
#define RK3288_PULL_PMU_OFFSET          0x64

static void rk3288_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The first 24 pins of the first bank are located in PMU */
	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = RK3288_PULL_PMU_OFFSET;

		*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);
		*bit = pin_num % ROCKCHIP_PULL_PINS_PER_REG;
		*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
	} else {
		*regmap = priv->regmap_base;
		*reg = RK3288_PULL_OFFSET;

		/* correct the offset, as we're starting with the 2nd bank */
		*reg -= 0x10;
		*reg += bank->bank_num * ROCKCHIP_PULL_BANK_STRIDE;
		*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);

		*bit = (pin_num % ROCKCHIP_PULL_PINS_PER_REG);
		*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
	}
}

#define RK3288_DRV_PMU_OFFSET		0x70
#define RK3288_DRV_GRF_OFFSET		0x1c0

static void rk3288_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The first 24 pins of the first bank are located in PMU */
	if (bank->bank_num == 0) {
		*regmap = priv->regmap_pmu;
		*reg = RK3288_DRV_PMU_OFFSET;

		*reg += ((pin_num / ROCKCHIP_DRV_PINS_PER_REG) * 4);
		*bit = pin_num % ROCKCHIP_DRV_PINS_PER_REG;
		*bit *= ROCKCHIP_DRV_BITS_PER_PIN;
	} else {
		*regmap = priv->regmap_base;
		*reg = RK3288_DRV_GRF_OFFSET;

		/* correct the offset, as we're starting with the 2nd bank */
		*reg -= 0x10;
		*reg += bank->bank_num * ROCKCHIP_DRV_BANK_STRIDE;
		*reg += ((pin_num / ROCKCHIP_DRV_PINS_PER_REG) * 4);

		*bit = (pin_num % ROCKCHIP_DRV_PINS_PER_REG);
		*bit *= ROCKCHIP_DRV_BITS_PER_PIN;
	}
}

static struct rockchip_pin_bank rk3288_pin_banks[] = {
	PIN_BANK_IOMUX_DRV_PULL_FLAGS(0, 24, "gpio0",
				      IOMUX_SOURCE_PMU | IOMUX_WRITABLE_32BIT,
				      IOMUX_SOURCE_PMU | IOMUX_WRITABLE_32BIT,
				      IOMUX_SOURCE_PMU | IOMUX_WRITABLE_32BIT,
				      IOMUX_UNROUTED,
				      DRV_TYPE_WRITABLE_32BIT,
				      DRV_TYPE_WRITABLE_32BIT,
				      DRV_TYPE_WRITABLE_32BIT,
				      0,
				      PULL_TYPE_WRITABLE_32BIT,
				      PULL_TYPE_WRITABLE_32BIT,
				      PULL_TYPE_WRITABLE_32BIT,
				      0
			    ),
	PIN_BANK_IOMUX_FLAGS(1, 32, "gpio1", IOMUX_UNROUTED,
					     IOMUX_UNROUTED,
					     IOMUX_UNROUTED,
					     0
			    ),
	PIN_BANK_IOMUX_FLAGS(2, 32, "gpio2", 0, 0, 0, IOMUX_UNROUTED),
	PIN_BANK_IOMUX_FLAGS(3, 32, "gpio3", 0, 0, 0, IOMUX_WIDTH_4BIT),
	PIN_BANK_IOMUX_FLAGS(4, 32, "gpio4", IOMUX_WIDTH_4BIT,
					     IOMUX_WIDTH_4BIT,
					     0,
					     0
			    ),
	PIN_BANK_IOMUX_FLAGS(5, 32, "gpio5", IOMUX_UNROUTED,
					     0,
					     0,
					     IOMUX_UNROUTED
			    ),
	PIN_BANK_IOMUX_FLAGS(6, 32, "gpio6", 0, 0, 0, IOMUX_UNROUTED),
	PIN_BANK_IOMUX_FLAGS(7, 32, "gpio7", 0,
					     0,
					     IOMUX_WIDTH_4BIT,
					     IOMUX_UNROUTED
			    ),
	PIN_BANK(8, 16, "gpio8"),
};

static struct rockchip_pin_ctrl rk3288_pin_ctrl = {
		.pin_banks		= rk3288_pin_banks,
		.nr_banks		= ARRAY_SIZE(rk3288_pin_banks),
		.label			= "RK3288-GPIO",
		.type			= RK3288,
		.grf_mux_offset		= 0x0,
		.pmu_mux_offset		= 0x84,
		.iomux_routes		= rk3288_mux_route_data,
		.niomux_routes		= ARRAY_SIZE(rk3288_mux_route_data),
		.pull_calc_reg		= rk3288_calc_pull_reg_and_bit,
		.drv_calc_reg		= rk3288_calc_drv_reg_and_bit,
};

static const struct udevice_id rk3288_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3288-pinctrl",
		.data = (ulong)&rk3288_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3288) = {
	.name		= "rockchip_rk3288_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3288_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

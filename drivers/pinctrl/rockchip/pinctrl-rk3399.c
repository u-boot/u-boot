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

static struct rockchip_mux_route_data rk3399_mux_route_data[] = {
	{
		/* uart2dbga_rx */
		.bank_num = 4,
		.pin = 8,
		.func = 2,
		.route_offset = 0xe21c,
		.route_val = BIT(16 + 10) | BIT(16 + 11),
	}, {
		/* uart2dbgb_rx */
		.bank_num = 4,
		.pin = 16,
		.func = 2,
		.route_offset = 0xe21c,
		.route_val = BIT(16 + 10) | BIT(16 + 11) | BIT(10),
	}, {
		/* uart2dbgc_rx */
		.bank_num = 4,
		.pin = 19,
		.func = 1,
		.route_offset = 0xe21c,
		.route_val = BIT(16 + 10) | BIT(16 + 11) | BIT(11),
	}, {
		/* pcie_clkreqn */
		.bank_num = 2,
		.pin = 26,
		.func = 2,
		.route_offset = 0xe21c,
		.route_val = BIT(16 + 14),
	}, {
		/* pcie_clkreqnb */
		.bank_num = 4,
		.pin = 24,
		.func = 1,
		.route_offset = 0xe21c,
		.route_val = BIT(16 + 14) | BIT(14),
	},
};

static int rk3399_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int iomux_num = (pin / 8);
	struct regmap *regmap;
	int reg, ret, mask, mux_type;
	u8 bit;
	u32 data, route_reg, route_val;

	regmap = (bank->iomux[iomux_num].type & IOMUX_SOURCE_PMU)
				? priv->regmap_pmu : priv->regmap_base;

	/* get basic quadrupel of mux registers and the correct reg inside */
	mux_type = bank->iomux[iomux_num].type;
	reg = bank->iomux[iomux_num].offset;
	reg += rockchip_get_mux_data(mux_type, pin, &bit, &mask);

	if (bank->route_mask & BIT(pin)) {
		if (rockchip_get_mux_route(bank, pin, mux, &route_reg,
					   &route_val)) {
			ret = regmap_write(regmap, route_reg, route_val);
			if (ret)
				return ret;
		}
	}

	data = (mask << (bit + 16));
	data |= (mux & mask) << bit;
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define RK3399_PULL_GRF_OFFSET		0xe040
#define RK3399_PULL_PMU_OFFSET		0x40

static void rk3399_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	/* The bank0:16 and bank1:32 pins are located in PMU */
	if (bank->bank_num == 0 || bank->bank_num == 1) {
		*regmap = priv->regmap_pmu;
		*reg = RK3399_PULL_PMU_OFFSET;

		*reg += bank->bank_num * ROCKCHIP_PULL_BANK_STRIDE;

		*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);
		*bit = pin_num % ROCKCHIP_PULL_PINS_PER_REG;
		*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
	} else {
		*regmap = priv->regmap_base;
		*reg = RK3399_PULL_GRF_OFFSET;

		/* correct the offset, as we're starting with the 3rd bank */
		*reg -= 0x20;
		*reg += bank->bank_num * ROCKCHIP_PULL_BANK_STRIDE;
		*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);

		*bit = (pin_num % ROCKCHIP_PULL_PINS_PER_REG);
		*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
	}
}

static void rk3399_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int drv_num = (pin_num / 8);

	/*  The bank0:16 and bank1:32 pins are located in PMU */
	if (bank->bank_num == 0 || bank->bank_num == 1)
		*regmap = priv->regmap_pmu;
	else
		*regmap = priv->regmap_base;

	*reg = bank->drv[drv_num].offset;
	if (bank->drv[drv_num].drv_type == DRV_TYPE_IO_1V8_3V0_AUTO ||
	    bank->drv[drv_num].drv_type == DRV_TYPE_IO_3V3_ONLY)
		*bit = (pin_num % 8) * 3;
	else
		*bit = (pin_num % 8) * 2;
}

static struct rockchip_pin_bank rk3399_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS_DRV_FLAGS_OFFSET_PULL_FLAGS(0, 32, "gpio0",
							 IOMUX_SOURCE_PMU,
							 IOMUX_SOURCE_PMU,
							 IOMUX_SOURCE_PMU,
							 IOMUX_SOURCE_PMU,
							 DRV_TYPE_IO_1V8_ONLY,
							 DRV_TYPE_IO_1V8_ONLY,
							 DRV_TYPE_IO_DEFAULT,
							 DRV_TYPE_IO_DEFAULT,
							 0x80,
							 0x88,
							 -1,
							 -1,
							 PULL_TYPE_IO_1V8_ONLY,
							 PULL_TYPE_IO_1V8_ONLY,
							 PULL_TYPE_IO_DEFAULT,
							 PULL_TYPE_IO_DEFAULT
							),
	PIN_BANK_IOMUX_DRV_FLAGS_OFFSET(1, 32, "gpio1", IOMUX_SOURCE_PMU,
					IOMUX_SOURCE_PMU,
					IOMUX_SOURCE_PMU,
					IOMUX_SOURCE_PMU,
					DRV_TYPE_IO_1V8_OR_3V0,
					DRV_TYPE_IO_1V8_OR_3V0,
					DRV_TYPE_IO_1V8_OR_3V0,
					DRV_TYPE_IO_1V8_OR_3V0,
					0xa0,
					0xa8,
					0xb0,
					0xb8
					),
	PIN_BANK_DRV_FLAGS_PULL_FLAGS(2, 32, "gpio2", DRV_TYPE_IO_1V8_OR_3V0,
				      DRV_TYPE_IO_1V8_OR_3V0,
				      DRV_TYPE_IO_1V8_ONLY,
				      DRV_TYPE_IO_1V8_ONLY,
				      PULL_TYPE_IO_DEFAULT,
				      PULL_TYPE_IO_DEFAULT,
				      PULL_TYPE_IO_1V8_ONLY,
				      PULL_TYPE_IO_1V8_ONLY
				      ),
	PIN_BANK_DRV_FLAGS(3, 32, "gpio3", DRV_TYPE_IO_3V3_ONLY,
			   DRV_TYPE_IO_3V3_ONLY,
			   DRV_TYPE_IO_3V3_ONLY,
			   DRV_TYPE_IO_1V8_OR_3V0
			   ),
	PIN_BANK_DRV_FLAGS(4, 32, "gpio4", DRV_TYPE_IO_1V8_OR_3V0,
			   DRV_TYPE_IO_1V8_3V0_AUTO,
			   DRV_TYPE_IO_1V8_OR_3V0,
			   DRV_TYPE_IO_1V8_OR_3V0
			   ),
};

static struct rockchip_pin_ctrl rk3399_pin_ctrl = {
	.pin_banks		= rk3399_pin_banks,
	.nr_banks		= ARRAY_SIZE(rk3399_pin_banks),
	.label			= "RK3399-GPIO",
	.type			= RK3399,
	.grf_mux_offset		= 0xe000,
	.pmu_mux_offset		= 0x0,
	.grf_drv_offset		= 0xe100,
	.pmu_drv_offset		= 0x80,
	.iomux_routes		= rk3399_mux_route_data,
	.niomux_routes		= ARRAY_SIZE(rk3399_mux_route_data),
	.set_mux		= rk3399_set_mux,
	.pull_calc_reg		= rk3399_calc_pull_reg_and_bit,
	.drv_calc_reg		= rk3399_calc_drv_reg_and_bit,
};

static const struct udevice_id rk3399_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3399-pinctrl",
		.data = (ulong)&rk3399_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3399) = {
	.name		= "rockchip_rk3399_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3399_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

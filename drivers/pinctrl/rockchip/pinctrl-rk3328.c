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

static struct rockchip_mux_recalced_data rk3328_mux_recalced_data[] = {
	{
		.num = 2,
		.pin = 12,
		.reg = 0x24,
		.bit = 8,
		.mask = 0x3
	}, {
		.num = 2,
		.pin = 15,
		.reg = 0x28,
		.bit = 0,
		.mask = 0x7
	}, {
		.num = 2,
		.pin = 23,
		.reg = 0x30,
		.bit = 14,
		.mask = 0x3
	},
};

static struct rockchip_mux_route_data rk3328_mux_route_data[] = {
	{
		/* uart2dbg_rxm0 */
		.bank_num = 1,
		.pin = 1,
		.func = 2,
		.route_offset = 0x50,
		.route_val = BIT(16) | BIT(16 + 1),
	}, {
		/* uart2dbg_rxm1 */
		.bank_num = 2,
		.pin = 1,
		.func = 1,
		.route_offset = 0x50,
		.route_val = BIT(16) | BIT(16 + 1) | BIT(0),
	}, {
		/* gmac-m1_rxd0 */
		.bank_num = 1,
		.pin = 11,
		.func = 2,
		.route_offset = 0x50,
		.route_val = BIT(16 + 2) | BIT(2),
	}, {
		/* gmac-m1-optimized_rxd3 */
		.bank_num = 1,
		.pin = 14,
		.func = 2,
		.route_offset = 0x50,
		.route_val = BIT(16 + 10) | BIT(10),
	}, {
		/* pdm_sdi0m0 */
		.bank_num = 2,
		.pin = 19,
		.func = 2,
		.route_offset = 0x50,
		.route_val = BIT(16 + 3),
	}, {
		/* pdm_sdi0m1 */
		.bank_num = 1,
		.pin = 23,
		.func = 3,
		.route_offset = 0x50,
		.route_val =  BIT(16 + 3) | BIT(3),
	}, {
		/* spi_rxdm2 */
		.bank_num = 3,
		.pin = 2,
		.func = 4,
		.route_offset = 0x50,
		.route_val =  BIT(16 + 4) | BIT(16 + 5) | BIT(5),
	}, {
		/* i2s2_sdim0 */
		.bank_num = 1,
		.pin = 24,
		.func = 1,
		.route_offset = 0x50,
		.route_val = BIT(16 + 6),
	}, {
		/* i2s2_sdim1 */
		.bank_num = 3,
		.pin = 2,
		.func = 6,
		.route_offset = 0x50,
		.route_val =  BIT(16 + 6) | BIT(6),
	}, {
		/* card_iom1 */
		.bank_num = 2,
		.pin = 22,
		.func = 3,
		.route_offset = 0x50,
		.route_val =  BIT(16 + 7) | BIT(7),
	}, {
		/* tsp_d5m1 */
		.bank_num = 2,
		.pin = 16,
		.func = 3,
		.route_offset = 0x50,
		.route_val =  BIT(16 + 8) | BIT(8),
	}, {
		/* cif_data5m1 */
		.bank_num = 2,
		.pin = 16,
		.func = 4,
		.route_offset = 0x50,
		.route_val =  BIT(16 + 9) | BIT(9),
	},
};

#define RK3328_PULL_OFFSET		0x100

static void rk3328_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	*reg = RK3328_PULL_OFFSET;
	*reg += bank->bank_num * ROCKCHIP_PULL_BANK_STRIDE;
	*reg += ((pin_num / ROCKCHIP_PULL_PINS_PER_REG) * 4);

	*bit = (pin_num % ROCKCHIP_PULL_PINS_PER_REG);
	*bit *= ROCKCHIP_PULL_BITS_PER_PIN;
}

#define RK3328_DRV_GRF_OFFSET		0x200

static void rk3328_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	*reg = RK3328_DRV_GRF_OFFSET;
	*reg += bank->bank_num * ROCKCHIP_DRV_BANK_STRIDE;
	*reg += ((pin_num / ROCKCHIP_DRV_PINS_PER_REG) * 4);

	*bit = (pin_num % ROCKCHIP_DRV_PINS_PER_REG);
	*bit *= ROCKCHIP_DRV_BITS_PER_PIN;
}

#define RK3328_SCHMITT_BITS_PER_PIN		1
#define RK3328_SCHMITT_PINS_PER_REG		16
#define RK3328_SCHMITT_BANK_STRIDE		8
#define RK3328_SCHMITT_GRF_OFFSET		0x380

static int rk3328_calc_schmitt_reg_and_bit(struct rockchip_pin_bank *bank,
					   int pin_num,
					   struct regmap **regmap,
					   int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	*reg = RK3328_SCHMITT_GRF_OFFSET;

	*reg += bank->bank_num * RK3328_SCHMITT_BANK_STRIDE;
	*reg += ((pin_num / RK3328_SCHMITT_PINS_PER_REG) * 4);
	*bit = pin_num % RK3328_SCHMITT_PINS_PER_REG;

	return 0;
}

static struct rockchip_pin_bank rk3328_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS(0, 32, "gpio0", 0, 0, 0, 0),
	PIN_BANK_IOMUX_FLAGS(1, 32, "gpio1", 0, 0, 0, 0),
	PIN_BANK_IOMUX_FLAGS(2, 32, "gpio2", 0,
			     IOMUX_WIDTH_3BIT,
			     IOMUX_WIDTH_3BIT,
			     0),
	PIN_BANK_IOMUX_FLAGS(3, 32, "gpio3",
			     IOMUX_WIDTH_3BIT,
			     IOMUX_WIDTH_3BIT,
			     0,
			     0),
};

static struct rockchip_pin_ctrl rk3328_pin_ctrl = {
		.pin_banks		= rk3328_pin_banks,
		.nr_banks		= ARRAY_SIZE(rk3328_pin_banks),
		.label			= "RK3328-GPIO",
		.type			= RK3288,
		.grf_mux_offset		= 0x0,
		.iomux_recalced		= rk3328_mux_recalced_data,
		.niomux_recalced	= ARRAY_SIZE(rk3328_mux_recalced_data),
		.iomux_routes		= rk3328_mux_route_data,
		.niomux_routes		= ARRAY_SIZE(rk3328_mux_route_data),
		.pull_calc_reg		= rk3328_calc_pull_reg_and_bit,
		.drv_calc_reg		= rk3328_calc_drv_reg_and_bit,
		.schmitt_calc_reg	= rk3328_calc_schmitt_reg_and_bit,
};

static const struct udevice_id rk3328_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3328-pinctrl",
		.data = (ulong)&rk3328_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3328) = {
	.name		= "rockchip_rk3328_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3328_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

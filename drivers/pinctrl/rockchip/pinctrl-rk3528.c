// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2022 Rockchip Electronics Co., Ltd.
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>

#include "pinctrl-rockchip.h"
#include <dt-bindings/pinctrl/rockchip.h>

static int rk3528_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int iomux_num = (pin / 8);
	struct regmap *regmap;
	int reg, mask;
	u8 bit;
	u32 data, rmask;

	regmap = priv->regmap_base;
	reg = bank->iomux[iomux_num].offset;
	if ((pin % 8) >= 4)
		reg += 0x4;
	bit = (pin % 4) * 4;
	mask = 0xf;

	data = (mask << (bit + 16));
	rmask = data | (data >> 16);
	data |= (mux & mask) << bit;

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3528_DRV_BITS_PER_PIN		8
#define RK3528_DRV_PINS_PER_REG		2
#define RK3528_DRV_GPIO0_OFFSET		0x100
#define RK3528_DRV_GPIO1_OFFSET		0x20120
#define RK3528_DRV_GPIO2_OFFSET		0x30160
#define RK3528_DRV_GPIO3_OFFSET		0x20190
#define RK3528_DRV_GPIO4_OFFSET		0x101C0

static void rk3528_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;

	if (bank->bank_num == 0) {
		*reg = RK3528_DRV_GPIO0_OFFSET;
	} else if (bank->bank_num == 1) {
		*reg = RK3528_DRV_GPIO1_OFFSET;
	} else if (bank->bank_num == 2) {
		*reg = RK3528_DRV_GPIO2_OFFSET;
	} else if (bank->bank_num == 3) {
		*reg = RK3528_DRV_GPIO3_OFFSET;
	} else if (bank->bank_num == 4) {
		*reg = RK3528_DRV_GPIO4_OFFSET;
	} else {
		*reg = 0;
		debug("unsupported bank_num %d\n", bank->bank_num);
	}

	*reg += ((pin_num / RK3528_DRV_PINS_PER_REG) * 4);
	*bit = pin_num % RK3528_DRV_PINS_PER_REG;
	*bit *= RK3528_DRV_BITS_PER_PIN;
}

static int rk3528_set_drive(struct rockchip_pin_bank *bank,
			    int pin_num, int strength)
{
	struct regmap *regmap;
	int reg;
	u32 data, rmask;
	u8 bit;
	int drv = (1 << (strength + 1)) - 1;

	rk3528_calc_drv_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3528_DRV_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (drv << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3528_PULL_BITS_PER_PIN		2
#define RK3528_PULL_PINS_PER_REG		8
#define RK3528_PULL_GPIO0_OFFSET		0x200
#define RK3528_PULL_GPIO1_OFFSET		0x20210
#define RK3528_PULL_GPIO2_OFFSET		0x30220
#define RK3528_PULL_GPIO3_OFFSET		0x20230
#define RK3528_PULL_GPIO4_OFFSET		0x10240

static void rk3528_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;

	if (bank->bank_num == 0) {
		*reg = RK3528_PULL_GPIO0_OFFSET;
	} else if (bank->bank_num == 1) {
		*reg = RK3528_PULL_GPIO1_OFFSET;
	} else if (bank->bank_num == 2) {
		*reg = RK3528_PULL_GPIO2_OFFSET;
	} else if (bank->bank_num == 3) {
		*reg = RK3528_PULL_GPIO3_OFFSET;
	} else if (bank->bank_num == 4) {
		*reg = RK3528_PULL_GPIO4_OFFSET;
	} else {
		*reg = 0;
		debug("unsupported bank_num %d\n", bank->bank_num);
	}

	*reg += ((pin_num / RK3528_PULL_PINS_PER_REG) * 4);
	*bit = pin_num % RK3528_PULL_PINS_PER_REG;
	*bit *= RK3528_PULL_BITS_PER_PIN;
}

static int rk3528_set_pull(struct rockchip_pin_bank *bank,
			   int pin_num, int pull)
{
	struct regmap *regmap;
	int reg, ret;
	u8 bit, type;
	u32 data, rmask;

	if (pull == PIN_CONFIG_BIAS_PULL_PIN_DEFAULT)
		return -EOPNOTSUPP;

	rk3528_calc_pull_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	type = bank->pull_type[pin_num / 8];
	ret = rockchip_translate_pull_value(type, pull);
	if (ret < 0) {
		debug("unsupported pull setting %d\n", pull);
		return ret;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3528_PULL_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (ret << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3528_SMT_BITS_PER_PIN		1
#define RK3528_SMT_PINS_PER_REG		8
#define RK3528_SMT_GPIO0_OFFSET		0x400
#define RK3528_SMT_GPIO1_OFFSET		0x20410
#define RK3528_SMT_GPIO2_OFFSET		0x30420
#define RK3528_SMT_GPIO3_OFFSET		0x20430
#define RK3528_SMT_GPIO4_OFFSET		0x10440

static int rk3528_calc_schmitt_reg_and_bit(struct rockchip_pin_bank *bank,
					   int pin_num,
					   struct regmap **regmap,
					   int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;

	if (bank->bank_num == 0) {
		*reg = RK3528_SMT_GPIO0_OFFSET;
	} else if (bank->bank_num == 1) {
		*reg = RK3528_SMT_GPIO1_OFFSET;
	} else if (bank->bank_num == 2) {
		*reg = RK3528_SMT_GPIO2_OFFSET;
	} else if (bank->bank_num == 3) {
		*reg = RK3528_SMT_GPIO3_OFFSET;
	} else if (bank->bank_num == 4) {
		*reg = RK3528_SMT_GPIO4_OFFSET;
	} else {
		*reg = 0;
		debug("unsupported bank_num %d\n", bank->bank_num);
	}

	*reg += ((pin_num / RK3528_SMT_PINS_PER_REG) * 4);
	*bit = pin_num % RK3528_SMT_PINS_PER_REG;
	*bit *= RK3528_SMT_BITS_PER_PIN;

	return 0;
}

static int rk3528_set_schmitt(struct rockchip_pin_bank *bank,
			      int pin_num, int enable)
{
	struct regmap *regmap;
	int reg;
	u32 data, rmask;
	u8 bit;

	rk3528_calc_schmitt_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3528_SMT_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (enable << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

static struct rockchip_pin_bank rk3528_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS_OFFSET(0, 32, "gpio0",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0, 0, 0, 0),
	PIN_BANK_IOMUX_FLAGS_OFFSET(1, 32, "gpio1",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x20020, 0x20028, 0x20030, 0x20038),
	PIN_BANK_IOMUX_FLAGS_OFFSET(2, 32, "gpio2",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x30040, 0, 0, 0),
	PIN_BANK_IOMUX_FLAGS_OFFSET(3, 32, "gpio3",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x20060, 0x20068, 0x20070, 0),
	PIN_BANK_IOMUX_FLAGS_OFFSET(4, 32, "gpio4",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x10080, 0x10088, 0x10090, 0x10098),
};

static const struct rockchip_pin_ctrl rk3528_pin_ctrl = {
	.pin_banks		= rk3528_pin_banks,
	.nr_banks		= ARRAY_SIZE(rk3528_pin_banks),
	.grf_mux_offset		= 0x0,
	.set_mux		= rk3528_set_mux,
	.set_pull		= rk3528_set_pull,
	.set_drive		= rk3528_set_drive,
	.set_schmitt		= rk3528_set_schmitt,
};

static const struct udevice_id rk3528_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3528-pinctrl",
		.data = (ulong)&rk3528_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(rockchip_rk3528_pinctrl) = {
	.name		= "rockchip_rk3528_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3528_pinctrl_ids,
	.priv_auto	= sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

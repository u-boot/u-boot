// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2024 Rockchip Electronics Co., Ltd.
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>

#include "pinctrl-rockchip.h"
#include <dt-bindings/pinctrl/rockchip.h>

static int rk3506_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int iomux_num = (pin / 8);
	struct regmap *regmap;
	int reg, mask;
	u8 bit;
	u32 data, rmask;

	if (bank->iomux[iomux_num].type & IOMUX_SOURCE_PMU)
		regmap = priv->regmap_pmu;
	else
		regmap = priv->regmap_base;

	if (bank->bank_num == 1)
		regmap = priv->regmap_ioc1;
	else if (bank->bank_num == 4)
		return 0;

	reg = bank->iomux[iomux_num].offset;
	if ((pin % 8) >= 4)
		reg += 0x4;
	bit = (pin % 4) * 4;
	mask = 0xf;

	if (bank->recalced_mask & BIT(pin))
		rockchip_get_recalced_mux(bank, pin, &reg, &bit, &mask);

	data = (mask << (bit + 16));
	rmask = data | (data >> 16);
	data |= (mux & mask) << bit;

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3506_DRV_BITS_PER_PIN		8
#define RK3506_DRV_PINS_PER_REG		2
#define RK3506_DRV_GPIO0_A_OFFSET	0x100
#define RK3506_DRV_GPIO0_D_OFFSET	0x830
#define RK3506_DRV_GPIO1_OFFSET		0x140
#define RK3506_DRV_GPIO2_OFFSET		0x180
#define RK3506_DRV_GPIO3_OFFSET		0x1c0
#define RK3506_DRV_GPIO4_OFFSET		0x840

static int rk3506_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
				       int pin_num, struct regmap **regmap,
				       int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int ret = 0;

	switch (bank->bank_num) {
	case 0:
		*regmap = priv->regmap_pmu;
		if (pin_num > 24) {
			ret = -EINVAL;
		} else if (pin_num < 24) {
			*reg = RK3506_DRV_GPIO0_A_OFFSET;
		} else {
			*reg = RK3506_DRV_GPIO0_D_OFFSET;
			*bit = 3;

			return 0;
		}
		break;

	case 1:
		*regmap = priv->regmap_ioc1;
		if (pin_num < 28)
			*reg = RK3506_DRV_GPIO1_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 2:
		*regmap = priv->regmap_base;
		if (pin_num < 17)
			*reg = RK3506_DRV_GPIO2_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 3:
		*regmap = priv->regmap_base;
		if (pin_num < 15)
			*reg = RK3506_DRV_GPIO3_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 4:
		*regmap = priv->regmap_base;
		if (pin_num < 8 || pin_num > 11) {
			ret = -EINVAL;
		} else {
			*reg = RK3506_DRV_GPIO4_OFFSET;
			*bit = 10;

			return 0;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		debug("unsupported bank_num %d pin_num %d\n", bank->bank_num, pin_num);
		return ret;
	}

	*reg += ((pin_num / RK3506_DRV_PINS_PER_REG) * 4);
	*bit = pin_num % RK3506_DRV_PINS_PER_REG;
	*bit *= RK3506_DRV_BITS_PER_PIN;

	return 0;
}

static int rk3506_set_drive(struct rockchip_pin_bank *bank,
			    int pin_num, int strength)
{
	struct regmap *regmap;
	int reg, ret, i;
	u32 data, rmask;
	u8 bit;
	int rmask_bits = RK3506_DRV_BITS_PER_PIN;

	ret = rk3506_calc_drv_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	if (ret)
		return ret;

	for (i = 0, ret = 1; i < strength; i++)
		ret = (ret << 1) | 1;

	if ((bank->bank_num == 0 && pin_num == 24) || bank->bank_num == 4) {
		rmask_bits = 2;
		ret = strength;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << rmask_bits) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (ret << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3506_PULL_BITS_PER_PIN	2
#define RK3506_PULL_PINS_PER_REG	8
#define RK3506_PULL_GPIO0_A_OFFSET	0x200
#define RK3506_PULL_GPIO0_D_OFFSET	0x830
#define RK3506_PULL_GPIO1_OFFSET	0x210
#define RK3506_PULL_GPIO2_OFFSET	0x220
#define RK3506_PULL_GPIO3_OFFSET	0x230
#define RK3506_PULL_GPIO4_OFFSET	0x840

static int rk3506_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int ret = 0;

	switch (bank->bank_num) {
	case 0:
		*regmap = priv->regmap_pmu;
		if (pin_num > 24) {
			ret = -EINVAL;
		} else if (pin_num < 24) {
			*reg = RK3506_PULL_GPIO0_A_OFFSET;
		} else {
			*reg = RK3506_PULL_GPIO0_D_OFFSET;
			*bit = 5;

			return 0;
		}
		break;

	case 1:
		*regmap = priv->regmap_ioc1;
		if (pin_num < 28)
			*reg = RK3506_PULL_GPIO1_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 2:
		*regmap = priv->regmap_base;
		if (pin_num < 17)
			*reg = RK3506_PULL_GPIO2_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 3:
		*regmap = priv->regmap_base;
		if (pin_num < 15)
			*reg = RK3506_PULL_GPIO3_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 4:
		*regmap = priv->regmap_base;
		if (pin_num < 8 || pin_num > 11) {
			ret = -EINVAL;
		} else {
			*reg = RK3506_PULL_GPIO4_OFFSET;
			*bit = 13;

			return 0;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		debug("unsupported bank_num %d pin_num %d\n", bank->bank_num, pin_num);
		return ret;
	}

	*reg += ((pin_num / RK3506_PULL_PINS_PER_REG) * 4);
	*bit = pin_num % RK3506_PULL_PINS_PER_REG;
	*bit *= RK3506_PULL_BITS_PER_PIN;

	return 0;
}

static int rk3506_set_pull(struct rockchip_pin_bank *bank,
			   int pin_num, int pull)
{
	struct regmap *regmap;
	int reg, ret;
	u8 bit, type;
	u32 data, rmask;

	if (pull == PIN_CONFIG_BIAS_PULL_PIN_DEFAULT)
		return -EOPNOTSUPP;

	ret = rk3506_calc_pull_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	if (ret)
		return ret;
	type = bank->pull_type[pin_num / 8];

	if ((bank->bank_num == 0 && pin_num == 24) || bank->bank_num == 4)
		type = 1;

	ret = rockchip_translate_pull_value(type, pull);
	if (ret < 0) {
		debug("unsupported pull setting %d\n", pull);
		return ret;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3506_PULL_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (ret << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3506_SMT_BITS_PER_PIN		1
#define RK3506_SMT_PINS_PER_REG		8
#define RK3506_SMT_GPIO0_A_OFFSET	0x400
#define RK3506_SMT_GPIO0_D_OFFSET	0x830
#define RK3506_SMT_GPIO1_OFFSET		0x410
#define RK3506_SMT_GPIO2_OFFSET		0x420
#define RK3506_SMT_GPIO3_OFFSET		0x430
#define RK3506_SMT_GPIO4_OFFSET		0x840

static int rk3506_calc_schmitt_reg_and_bit(struct rockchip_pin_bank *bank,
					   int pin_num,
					   struct regmap **regmap,
					   int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int ret = 0;

	switch (bank->bank_num) {
	case 0:
		*regmap = priv->regmap_pmu;
		if (pin_num > 24) {
			ret = -EINVAL;
		} else if (pin_num < 24) {
			*reg = RK3506_SMT_GPIO0_A_OFFSET;
		} else {
			*reg = RK3506_SMT_GPIO0_D_OFFSET;
			*bit = 9;

			return 0;
		}
		break;

	case 1:
		*regmap = priv->regmap_ioc1;
		if (pin_num < 28)
			*reg = RK3506_SMT_GPIO1_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 2:
		*regmap = priv->regmap_base;
		if (pin_num < 17)
			*reg = RK3506_SMT_GPIO2_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 3:
		*regmap = priv->regmap_base;
		if (pin_num < 15)
			*reg = RK3506_SMT_GPIO3_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 4:
		*regmap = priv->regmap_base;
		if (pin_num < 8 || pin_num > 11) {
			ret = -EINVAL;
		} else {
			*reg = RK3506_SMT_GPIO4_OFFSET;
			*bit = 8;

			return 0;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		debug("unsupported bank_num %d pin_num %d\n", bank->bank_num, pin_num);
		return ret;
	}

	*reg += ((pin_num / RK3506_SMT_PINS_PER_REG) * 4);
	*bit = pin_num % RK3506_SMT_PINS_PER_REG;
	*bit *= RK3506_SMT_BITS_PER_PIN;

	return 0;
}

static int rk3506_set_schmitt(struct rockchip_pin_bank *bank,
			      int pin_num, int enable)
{
	struct regmap *regmap;
	int reg, ret;
	u32 data, rmask;
	u8 bit;

	ret = rk3506_calc_schmitt_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	if (ret)
		return ret;

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3506_SMT_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (enable << bit);

	if ((bank->bank_num == 0 && pin_num == 24) || bank->bank_num == 4) {
		data = 0x3 << (bit + 16);
		rmask = data | (data >> 16);
		data |= ((enable ? 0x3 : 0) << bit);
	}

	return regmap_update_bits(regmap, reg, rmask, data);
}

static struct rockchip_mux_recalced_data rk3506_mux_recalced_data[] = {
	{
		.num = 0,
		.pin = 24,
		.reg = 0x830,
		.bit = 0,
		.mask = 0x3
	},
};

static struct rockchip_pin_bank rk3506_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS_OFFSET(0, 32, "gpio0",
				    IOMUX_WIDTH_4BIT | IOMUX_SOURCE_PMU,
				    IOMUX_WIDTH_4BIT | IOMUX_SOURCE_PMU,
				    IOMUX_WIDTH_4BIT | IOMUX_SOURCE_PMU,
				    IOMUX_8WIDTH_2BIT | IOMUX_SOURCE_PMU,
				    0x0, 0x8, 0x10, 0x830),
	PIN_BANK_IOMUX_FLAGS_OFFSET(1, 32, "gpio1",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x20, 0x28, 0x30, 0x38),
	PIN_BANK_IOMUX_FLAGS_OFFSET(2, 32, "gpio2",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x40, 0x48, 0x50, 0x58),
	PIN_BANK_IOMUX_FLAGS_OFFSET(3, 32, "gpio3",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x60, 0x68, 0x70, 0x78),
	PIN_BANK_IOMUX_FLAGS_OFFSET(4, 32, "gpio4",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x80, 0x88, 0x90, 0x98),
};

static const struct rockchip_pin_ctrl rk3506_pin_ctrl = {
	.pin_banks		= rk3506_pin_banks,
	.nr_banks		= ARRAY_SIZE(rk3506_pin_banks),
	.iomux_recalced		= rk3506_mux_recalced_data,
	.niomux_recalced	= ARRAY_SIZE(rk3506_mux_recalced_data),
	.set_mux		= rk3506_set_mux,
	.set_pull		= rk3506_set_pull,
	.set_drive		= rk3506_set_drive,
	.set_schmitt		= rk3506_set_schmitt,
};

static const struct udevice_id rk3506_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3506-pinctrl",
		.data = (ulong)&rk3506_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(rockchip_rk3506_pinctrl) = {
	.name		= "rockchip_rk3506_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3506_pinctrl_ids,
	.priv_auto	= sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

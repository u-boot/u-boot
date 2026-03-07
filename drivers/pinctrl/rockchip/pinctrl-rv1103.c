// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <log.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>
#include <linux/bitops.h>

#include "pinctrl-rockchip.h"

static int rv1103_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int iomux_num = (pin / 8);
	struct regmap *regmap;
	int reg, ret, mask;
	u8 bit;
	u32 data;

	debug("setting mux of GPIO%d-%d to %d\n", bank->bank_num, pin, mux);

	if (bank->bank_num == 2 && pin >= 12)
		return 0;

	regmap = priv->regmap_base;
	reg = bank->iomux[iomux_num].offset;
	if ((pin % 8) >= 4)
		reg += 0x4;
	bit = (pin % 4) * 4;
	mask = 0xf;

	if (bank->recalced_mask & BIT(pin))
		rockchip_get_recalced_mux(bank, pin, &reg, &bit, &mask);
	data = (mask << (bit + 16));
	data |= (mux & mask) << bit;

	debug("iomux write reg = %x data = %x\n", reg, data);

	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define RV1103_DRV_BITS_PER_PIN		8
#define RV1103_DRV_PINS_PER_REG		2
#define RV1103_DRV_GPIO0_A_OFFSET		0x40100
#define RV1103_DRV_GPIO0_B_OFFSET		0x50110
#define RV1103_DRV_GPIO1_A01_OFFSET		0x140
#define RV1103_DRV_GPIO1_A67_OFFSET		0x1014C
#define RV1103_DRV_GPIO2_OFFSET		0x30180
#define RV1103_DRV_GPIO2_SARADC_OFFSET		0x3080C

static int rv1103_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
				       int pin_num, struct regmap **regmap,
				       int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int ret = 0;

	*regmap = priv->regmap_base;
	switch (bank->bank_num) {
	case 0:
		if (pin_num < 7)
			*reg = RV1103_DRV_GPIO0_A_OFFSET;
		else if (pin_num > 7 && pin_num < 14)
			*reg = RV1103_DRV_GPIO0_B_OFFSET - 0x10;
		else
			ret = -EINVAL;
		break;

	case 1:
		if (pin_num < 6)
			*reg = RV1103_DRV_GPIO1_A01_OFFSET;
		else if (pin_num >= 6 && pin_num < 23)
			*reg = RV1103_DRV_GPIO1_A67_OFFSET - 0xc;
		else if (pin_num >= 24 && pin_num < 30)
			*reg = RV1103_DRV_GPIO1_A67_OFFSET - 0xc;
		else
			ret = -EINVAL;
		break;

	case 2:
		if (pin_num < 12) {
			*reg = RV1103_DRV_GPIO2_OFFSET;
		} else if (pin_num >= 16) {
			ret = -EINVAL;
		} else {
			*reg = RV1103_DRV_GPIO2_SARADC_OFFSET;
			*bit = 10;

			return 0;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		printf("unsupported bank_num %d pin_num %d\n", bank->bank_num, pin_num);

		return ret;
	}

	*reg += ((pin_num / RV1103_DRV_PINS_PER_REG) * 4);
	*bit = pin_num % RV1103_DRV_PINS_PER_REG;
	*bit *= RV1103_DRV_BITS_PER_PIN;

	return 0;
}

static int rv1103_set_drive(struct rockchip_pin_bank *bank,
			    int pin_num, int strength)
{
	struct regmap *regmap;
	int reg, ret, i;
	u32 data;
	u8 bit;
	int rmask_bits = RV1103_DRV_BITS_PER_PIN;

	ret = rv1103_calc_drv_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	if (ret)
		return ret;

	for (i = 0, ret = 1; i < strength; i++)
		ret = (ret << 1) | 1;

	if (bank->bank_num == 2 && pin_num >= 12) {
		rmask_bits = 2;
		ret = strength;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << rmask_bits) - 1) << (bit + 16);
	data |= (ret << bit);
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define RV1103_PULL_BITS_PER_PIN		2
#define RV1103_PULL_PINS_PER_REG		8
#define RV1103_PULL_GPIO0_A_OFFSET		0x40200
#define RV1103_PULL_GPIO0_B_OFFSET		0x50204
#define RV1103_PULL_GPIO1_A01_OFFSET		0x210
#define RV1103_PULL_GPIO1_A67_OFFSET		0x10210
#define RV1103_PULL_GPIO2_OFFSET		0x30220
#define RV1103_PULL_GPIO2_SARADC_OFFSET	0x3080C

static int rv1103_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int ret = 0;

	*regmap = priv->regmap_base;
	switch (bank->bank_num) {
	case 0:
		if (pin_num < 7)
			*reg = RV1103_PULL_GPIO0_A_OFFSET;
		else if (pin_num > 7 && pin_num < 14)
			*reg = RV1103_PULL_GPIO0_B_OFFSET - 0x4;
		else
			ret = -EINVAL;
		break;

	case 1:
		if (pin_num < 6)
			*reg = RV1103_PULL_GPIO1_A01_OFFSET;
		else if (pin_num >= 6 && pin_num < 23)
			*reg = RV1103_PULL_GPIO1_A67_OFFSET;
		else if (pin_num >= 24 && pin_num < 30)
			*reg = RV1103_PULL_GPIO1_A67_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 2:
		if (pin_num < 12) {
			*reg = RV1103_PULL_GPIO2_OFFSET;
		} else if (pin_num >= 16) {
			ret = -EINVAL;
		} else {
			*reg = RV1103_PULL_GPIO2_SARADC_OFFSET;
			*bit = 13;

			return 0;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		printf("unsupported bank_num %d pin_num %d\n", bank->bank_num, pin_num);

		return ret;
	}

	*reg += ((pin_num / RV1103_PULL_PINS_PER_REG) * 4);
	*bit = pin_num % RV1103_PULL_PINS_PER_REG;
	*bit *= RV1103_PULL_BITS_PER_PIN;

	return 0;
}

static int rv1103_set_pull(struct rockchip_pin_bank *bank,
			   int pin_num, int pull)
{
	struct regmap *regmap;
	int reg, ret;
	u8 bit, type;
	u32 data;

	if (pull == PIN_CONFIG_BIAS_PULL_PIN_DEFAULT)
		return -EINVAL;

	ret = rv1103_calc_pull_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	if (ret)
		return ret;
	type = bank->pull_type[pin_num / 8];

	if (bank->bank_num == 2 && pin_num >= 12)
		type = 1;

	ret = rockchip_translate_pull_value(type, pull);
	if (ret < 0) {
		debug("unsupported pull setting %d\n", pull);

		return ret;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << RV1103_PULL_BITS_PER_PIN) - 1) << (bit + 16);

	data |= (ret << bit);
	ret = regmap_write(regmap, reg, data);

	return ret;
}

#define RV1103_SMT_BITS_PER_PIN		1
#define RV1103_SMT_PINS_PER_REG		8
#define RV1103_SMT_GPIO0_A_OFFSET		0x40400
#define RV1103_SMT_GPIO0_B_OFFSET		0x50404
#define RV1103_SMT_GPIO1_A01_OFFSET		0x410
#define RV1103_SMT_GPIO1_A67_OFFSET		0x10410
#define RV1103_SMT_GPIO2_OFFSET		0x30420
#define RV1103_SMT_GPIO2_SARADC_OFFSET		0x3080C

static int rv1103_calc_schmitt_reg_and_bit(struct rockchip_pin_bank *bank,
					   int pin_num,
					   struct regmap **regmap,
					   int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;
	int ret = 0;

	*regmap = priv->regmap_base;
	switch (bank->bank_num) {
	case 0:
		if (pin_num < 7)
			*reg = RV1103_SMT_GPIO0_A_OFFSET;
		else if (pin_num > 7 && pin_num < 14)
			*reg = RV1103_SMT_GPIO0_B_OFFSET - 0x4;
		else
			ret = -EINVAL;
		break;

	case 1:
		if (pin_num < 6)
			*reg = RV1103_SMT_GPIO1_A01_OFFSET;
		else if (pin_num >= 6 && pin_num < 23)
			*reg = RV1103_SMT_GPIO1_A67_OFFSET;
		else if (pin_num >= 24 && pin_num < 30)
			*reg = RV1103_SMT_GPIO1_A67_OFFSET;
		else
			ret = -EINVAL;
		break;

	case 2:
		if (pin_num < 12) {
			*reg = RV1103_SMT_GPIO2_OFFSET;
		} else if (pin_num >= 16) {
			ret = -EINVAL;
		} else {
			*reg = RV1103_SMT_GPIO2_SARADC_OFFSET;
			*bit = 8;

			return 0;
		}
		break;

	default:
		ret = -EINVAL;
		break;
	}

	if (ret) {
		printf("unsupported bank_num %d pin_num %d\n", bank->bank_num, pin_num);

		return ret;
	}

	*reg += ((pin_num / RV1103_SMT_PINS_PER_REG) * 4);
	*bit = pin_num % RV1103_SMT_PINS_PER_REG;
	*bit *= RV1103_SMT_BITS_PER_PIN;

	return 0;
}

static int rv1103_set_schmitt(struct rockchip_pin_bank *bank,
			      int pin_num, int enable)
{
	struct regmap *regmap;
	int reg, ret;
	u32 data;
	u8 bit;

	ret = rv1103_calc_schmitt_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	if (ret)
		return ret;

	/* enable the write to the equivalent lower bits */
	data = ((1 << RV1103_SMT_BITS_PER_PIN) - 1) << (bit + 16);
	data |= (enable << bit);

	if (bank->bank_num == 2 && pin_num >= 12) {
		data = 0x3 << (bit + 16);
		data |= ((enable ? 0x3 : 0) << bit);
	}
	ret = regmap_write(regmap, reg, data);

	return ret;
}

static struct rockchip_mux_recalced_data rv1103_mux_recalced_data[] = {
	{
		.num = 1,
		.pin = 6,
		.reg = 0x10024,
		.bit = 8,
		.mask = 0xf
	}, {
		.num = 1,
		.pin = 7,
		.reg = 0x10024,
		.bit = 12,
		.mask = 0xf
	},
};

static struct rockchip_pin_bank rv1103_pin_banks[] = {
	PIN_BANK_IOMUX_FLAGS_OFFSET(0, 32, "gpio0",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x40000, 0x50008, 0x50010, 0x50018),
	PIN_BANK_IOMUX_FLAGS_OFFSET(1, 32, "gpio1",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x20, 0x10028, 0x10030, 0x10038),
	PIN_BANK_IOMUX_FLAGS_OFFSET(2, 32, "gpio2",
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    IOMUX_WIDTH_4BIT,
				    0x30040, 0x30048, 0x30050, 0x30058),
};

static const struct rockchip_pin_ctrl rv1103_pin_ctrl = {
	.pin_banks		= rv1103_pin_banks,
	.nr_banks		= ARRAY_SIZE(rv1103_pin_banks),
	.iomux_recalced		= rv1103_mux_recalced_data,
	.niomux_recalced	= ARRAY_SIZE(rv1103_mux_recalced_data),
	.set_mux		= rv1103_set_mux,
	.set_pull		= rv1103_set_pull,
	.set_drive		= rv1103_set_drive,
	.set_schmitt		= rv1103_set_schmitt,
};

static const struct udevice_id rv1103_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rv1103-pinctrl",
		.data = (ulong)&rv1103_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_rv1103) = {
	.name		= "rockchip_rv1103_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rv1103_pinctrl_ids,
	.priv_auto	= sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

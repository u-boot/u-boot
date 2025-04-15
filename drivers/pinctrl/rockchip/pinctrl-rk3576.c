// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <dm/pinctrl.h>
#include <regmap.h>
#include <syscon.h>

#include "pinctrl-rockchip.h"
#include <dt-bindings/pinctrl/rockchip.h>

static int rk3576_set_mux(struct rockchip_pin_bank *bank, int pin, int mux)
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

	if (bank->bank_num == 0 && pin >= RK_PB4 && pin <= RK_PB7)
		reg += 0x1FF4; /* GPIO0_IOC_GPIO0B_IOMUX_SEL_H */

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3576_DRV_BITS_PER_PIN		4
#define RK3576_DRV_PINS_PER_REG		4
#define RK3576_DRV_GPIO0_AL_OFFSET	0x10
#define RK3576_DRV_GPIO0_BH_OFFSET	0x2014
#define RK3576_DRV_GPIO1_OFFSET		0x6020
#define RK3576_DRV_GPIO2_OFFSET		0x6040
#define RK3576_DRV_GPIO3_OFFSET		0x6060
#define RK3576_DRV_GPIO4_AL_OFFSET	0x6080
#define RK3576_DRV_GPIO4_CL_OFFSET	0xA090
#define RK3576_DRV_GPIO4_DL_OFFSET	0xB098

static void rk3576_calc_drv_reg_and_bit(struct rockchip_pin_bank *bank,
					int pin_num, struct regmap **regmap,
					int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	if (bank->bank_num == 0 && pin_num < 12) {
		*reg = RK3576_DRV_GPIO0_AL_OFFSET;
	} else if (bank->bank_num == 0) {
		*reg = RK3576_DRV_GPIO0_BH_OFFSET - 0xc;
	} else if (bank->bank_num == 1) {
		*reg = RK3576_DRV_GPIO1_OFFSET;
	} else if (bank->bank_num == 2) {
		*reg = RK3576_DRV_GPIO2_OFFSET;
	} else if (bank->bank_num == 3) {
		*reg = RK3576_DRV_GPIO3_OFFSET;
	} else if (bank->bank_num == 4 && pin_num < 16) {
		*reg = RK3576_DRV_GPIO4_AL_OFFSET;
	} else if (bank->bank_num == 4 && pin_num < 24) {
		*reg = RK3576_DRV_GPIO4_CL_OFFSET - 0x10;
	} else if (bank->bank_num == 4) {
		*reg = RK3576_DRV_GPIO4_DL_OFFSET - 0x18;
	} else {
		*reg = 0;
		debug("unsupported bank_num %d\n", bank->bank_num);
	}

	*reg += ((pin_num / RK3576_DRV_PINS_PER_REG) * 4);
	*bit = pin_num % RK3576_DRV_PINS_PER_REG;
	*bit *= RK3576_DRV_BITS_PER_PIN;
}

static int rk3576_set_drive(struct rockchip_pin_bank *bank,
			    int pin_num, int strength)
{
	struct regmap *regmap;
	int reg;
	u32 data, rmask;
	u8 bit;
	int drv = ((strength & BIT(2)) >> 2) | ((strength & BIT(0)) << 2) | (strength & BIT(1));

	rk3576_calc_drv_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3576_DRV_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (drv << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3576_PULL_BITS_PER_PIN	2
#define RK3576_PULL_PINS_PER_REG	8
#define RK3576_PULL_GPIO0_AL_OFFSET	0x20
#define RK3576_PULL_GPIO0_BH_OFFSET	0x2028
#define RK3576_PULL_GPIO1_OFFSET	0x6110
#define RK3576_PULL_GPIO2_OFFSET	0x6120
#define RK3576_PULL_GPIO3_OFFSET	0x6130
#define RK3576_PULL_GPIO4_AL_OFFSET	0x6140
#define RK3576_PULL_GPIO4_CL_OFFSET	0xA148
#define RK3576_PULL_GPIO4_DL_OFFSET	0xB14C

static void rk3576_calc_pull_reg_and_bit(struct rockchip_pin_bank *bank,
					 int pin_num, struct regmap **regmap,
					 int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	if (bank->bank_num == 0 && pin_num < 12) {
		*reg = RK3576_PULL_GPIO0_AL_OFFSET;
	} else if (bank->bank_num == 0) {
		*reg = RK3576_PULL_GPIO0_BH_OFFSET - 0x4;
	} else if (bank->bank_num == 1) {
		*reg = RK3576_PULL_GPIO1_OFFSET;
	} else if (bank->bank_num == 2) {
		*reg = RK3576_PULL_GPIO2_OFFSET;
	} else if (bank->bank_num == 3) {
		*reg = RK3576_PULL_GPIO3_OFFSET;
	} else if (bank->bank_num == 4 && pin_num < 16) {
		*reg = RK3576_PULL_GPIO4_AL_OFFSET;
	} else if (bank->bank_num == 4 && pin_num < 24) {
		*reg = RK3576_PULL_GPIO4_CL_OFFSET - 0x8;
	} else if (bank->bank_num == 4) {
		*reg = RK3576_PULL_GPIO4_DL_OFFSET - 0xc;
	} else {
		*reg = 0;
		debug("unsupported bank_num %d\n", bank->bank_num);
	}

	*reg += ((pin_num / RK3576_PULL_PINS_PER_REG) * 4);
	*bit = pin_num % RK3576_PULL_PINS_PER_REG;
	*bit *= RK3576_PULL_BITS_PER_PIN;
}

static int rk3576_set_pull(struct rockchip_pin_bank *bank,
			   int pin_num, int pull)
{
	struct regmap *regmap;
	int reg, ret;
	u8 bit, type;
	u32 data, rmask;

	if (pull == PIN_CONFIG_BIAS_PULL_PIN_DEFAULT)
		return -ENOTSUPP;

	rk3576_calc_pull_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);
	type = 1; /* FIXME: was always set to 1 in vendor kernel */
	ret = rockchip_translate_pull_value(type, pull);
	if (ret < 0) {
		debug("unsupported pull setting %d\n", pull);
		return ret;
	}

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3576_PULL_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (ret << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

#define RK3576_SMT_BITS_PER_PIN		1
#define RK3576_SMT_PINS_PER_REG		8
#define RK3576_SMT_GPIO0_AL_OFFSET	0x30
#define RK3576_SMT_GPIO0_BH_OFFSET	0x2040
#define RK3576_SMT_GPIO1_OFFSET		0x6210
#define RK3576_SMT_GPIO2_OFFSET		0x6220
#define RK3576_SMT_GPIO3_OFFSET		0x6230
#define RK3576_SMT_GPIO4_AL_OFFSET	0x6240
#define RK3576_SMT_GPIO4_CL_OFFSET	0xA248
#define RK3576_SMT_GPIO4_DL_OFFSET	0xB24C

static void rk3576_calc_schmitt_reg_and_bit(struct rockchip_pin_bank *bank,
					    int pin_num,
					    struct regmap **regmap,
					    int *reg, u8 *bit)
{
	struct rockchip_pinctrl_priv *priv = bank->priv;

	*regmap = priv->regmap_base;
	if (bank->bank_num == 0 && pin_num < 12) {
		*reg = RK3576_SMT_GPIO0_AL_OFFSET;
	} else if (bank->bank_num == 0) {
		*reg = RK3576_SMT_GPIO0_BH_OFFSET - 0x4;
	} else if (bank->bank_num == 1) {
		*reg = RK3576_SMT_GPIO1_OFFSET;
	} else if (bank->bank_num == 2) {
		*reg = RK3576_SMT_GPIO2_OFFSET;
	} else if (bank->bank_num == 3) {
		*reg = RK3576_SMT_GPIO3_OFFSET;
	} else if (bank->bank_num == 4 && pin_num < 16) {
		*reg = RK3576_SMT_GPIO4_AL_OFFSET;
	} else if (bank->bank_num == 4 && pin_num < 24) {
		*reg = RK3576_SMT_GPIO4_CL_OFFSET - 0x8;
	} else if (bank->bank_num == 4) {
		*reg = RK3576_SMT_GPIO4_DL_OFFSET - 0xc;
	} else {
		*reg = 0;
		debug("unsupported bank_num %d\n", bank->bank_num);
	}

	*reg += ((pin_num / RK3576_SMT_PINS_PER_REG) * 4);
	*bit = pin_num % RK3576_SMT_PINS_PER_REG;
	*bit *= RK3576_SMT_BITS_PER_PIN;
}

static int rk3576_set_schmitt(struct rockchip_pin_bank *bank,
			      int pin_num, int enable)
{
	struct regmap *regmap;
	int reg;
	u32 data, rmask;
	u8 bit;

	rk3576_calc_schmitt_reg_and_bit(bank, pin_num, &regmap, &reg, &bit);

	/* enable the write to the equivalent lower bits */
	data = ((1 << RK3576_SMT_BITS_PER_PIN) - 1) << (bit + 16);
	rmask = data | (data >> 16);
	data |= (enable << bit);

	return regmap_update_bits(regmap, reg, rmask, data);
}

static struct rockchip_pin_bank rk3576_pin_banks[] = {
	RK3576_PIN_BANK_FLAGS(0, 32, "gpio0", IOMUX_WIDTH_4BIT,
			      0, 0x8, 0x2004, 0x200C),
	RK3576_PIN_BANK_FLAGS(1, 32, "gpio1", IOMUX_WIDTH_4BIT,
			      0x4020, 0x4028, 0x4030, 0x4038),
	RK3576_PIN_BANK_FLAGS(2, 32, "gpio2", IOMUX_WIDTH_4BIT,
			      0x4040, 0x4048, 0x4050, 0x4058),
	RK3576_PIN_BANK_FLAGS(3, 32, "gpio3", IOMUX_WIDTH_4BIT,
			      0x4060, 0x4068, 0x4070, 0x4078),
	RK3576_PIN_BANK_FLAGS(4, 32, "gpio4", IOMUX_WIDTH_4BIT,
			      0x4080, 0x4088, 0xA390, 0xB398),
};

static const struct rockchip_pin_ctrl rk3576_pin_ctrl = {
	.pin_banks		= rk3576_pin_banks,
	.nr_banks		= ARRAY_SIZE(rk3576_pin_banks),
	.grf_mux_offset		= 0x0,
	.set_mux		= rk3576_set_mux,
	.set_pull		= rk3576_set_pull,
	.set_drive		= rk3576_set_drive,
	.set_schmitt		= rk3576_set_schmitt,
};

static const struct udevice_id rk3576_pinctrl_ids[] = {
	{
		.compatible = "rockchip,rk3576-pinctrl",
		.data = (ulong)&rk3576_pin_ctrl
	},
	{ }
};

U_BOOT_DRIVER(pinctrl_rk3576) = {
	.name		= "rockchip_rk3576_pinctrl",
	.id		= UCLASS_PINCTRL,
	.of_match	= rk3576_pinctrl_ids,
	.priv_auto	= sizeof(struct rockchip_pinctrl_priv),
	.ops		= &rockchip_pinctrl_ops,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind		= dm_scan_fdt_dev,
#endif
	.probe		= rockchip_pinctrl_probe,
};

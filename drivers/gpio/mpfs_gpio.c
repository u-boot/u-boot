// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2025 Microchip Technology Inc.
 * Eoin Dickson <eoin.dickson@microchip.com>
 */

#include <dm.h>
#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <errno.h>
#include <asm/gpio.h>
#include <linux/bitops.h>

#define MPFS_INP_REG			0x84
#define COREGPIO_INP_REG		0x90
#define MPFS_OUTP_REG			0x88
#define COREGPIO_OUTP_REG		0xA0
#define MPFS_GPIO_CTRL(i)		(0x4 * (i))
#define MPFS_MAX_NUM_GPIO		32
#define MPFS_GPIO_EN_OUT_BUF		BIT(2)
#define MPFS_GPIO_EN_IN			BIT(1)
#define MPFS_GPIO_EN_OUT		BIT(0)

struct mpfs_gpio_reg_offsets {
	u8 inp;
	u8 outp;
};

struct mchp_gpio_plat {
	void *base;
	const struct mpfs_gpio_reg_offsets *regs;
};

static void mchp_update_gpio_reg(void *bptr, u32 offset, bool value)
{
	void __iomem *ptr = (void __iomem *)bptr;

	u32 old = readl(ptr);

	if (value)
		writel(old | offset, ptr);
	else
		writel(old & ~offset, ptr);
}

static int mchp_gpio_direction_input(struct udevice *dev, u32 offset)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	mchp_update_gpio_reg(plat->base + MPFS_GPIO_CTRL(offset),  MPFS_GPIO_EN_IN, true);
	mchp_update_gpio_reg(plat->base + MPFS_GPIO_CTRL(offset),  MPFS_GPIO_EN_OUT, false);
	mchp_update_gpio_reg(plat->base + MPFS_GPIO_CTRL(offset),  MPFS_GPIO_EN_OUT_BUF, false);

	return 0;
}

static int mchp_gpio_direction_output(struct udevice *dev, u32 offset, int value)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	mchp_update_gpio_reg(plat->base + MPFS_GPIO_CTRL(offset),  MPFS_GPIO_EN_IN, false);
	mchp_update_gpio_reg(plat->base + MPFS_GPIO_CTRL(offset),  MPFS_GPIO_EN_OUT, true);
	mchp_update_gpio_reg(plat->base + MPFS_GPIO_CTRL(offset),  MPFS_GPIO_EN_OUT_BUF, true);

	mchp_update_gpio_reg(plat->base + plat->regs->outp, BIT(offset), value);

	return 0;
}

static bool mchp_gpio_get_value(struct udevice *dev, u32 offset)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int val, input;

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	input = readl(plat->base + MPFS_GPIO_CTRL(offset)) & MPFS_GPIO_EN_IN;

	if (input)
		val = (readl(plat->base + plat->regs->inp) & BIT(offset));
	else
		val = (readl(plat->base + plat->regs->outp) & BIT(offset));

	return val >> offset;
}

static int mchp_gpio_set_value(struct udevice *dev, u32 offset, int value)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	mchp_update_gpio_reg(plat->base + plat->regs->outp, BIT(offset), value);

	return 0;
}

static int mchp_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	u32	outdir, indir, val;

	if (offset > uc_priv->gpio_count)
		return -EINVAL;

	/* Get direction of the pin */
	outdir = readl(plat->base + MPFS_GPIO_CTRL(offset)) & MPFS_GPIO_EN_OUT;
	indir  = readl(plat->base + MPFS_GPIO_CTRL(offset)) & MPFS_GPIO_EN_IN;

	if (outdir)
		val = GPIOF_OUTPUT;
	else if (indir)
		val = GPIOF_INPUT;
	else
		val = GPIOF_UNUSED;

	return val;
}

static int mchp_gpio_probe(struct udevice *dev)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	char name[18], *str;

	plat->regs = dev_get_driver_data(dev);
	sprintf(name, "gpio@%4lx_", (uintptr_t)plat->base);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;
	uc_priv->gpio_count = dev_read_u32_default(dev, "ngpios", MPFS_MAX_NUM_GPIO);

	return 0;
}

static const struct mpfs_gpio_reg_offsets mpfs_reg_offsets = {
	.inp = MPFS_INP_REG,
	.outp = MPFS_OUTP_REG,
};

static const struct mpfs_gpio_reg_offsets coregpio_reg_offsets = {
	.inp = COREGPIO_INP_REG,
	.outp = COREGPIO_OUTP_REG,
};

static const struct udevice_id mchp_gpio_match[] = {
	{
		.compatible = "microchip,mpfs-gpio",
		.data = &mpfs_reg_offsets,
	}, {
		.compatible = "microchip,coregpio-rtl-v3",
		.data = &coregpio_reg_offsets,
	},
	{ /* end of list */ }
};

static const struct dm_gpio_ops mchp_gpio_ops = {
	.direction_input        = mchp_gpio_direction_input,
	.direction_output       = mchp_gpio_direction_output,
	.get_value              = mchp_gpio_get_value,
	.set_value              = mchp_gpio_set_value,
	.get_function		= mchp_gpio_get_function,
};

static int mchp_gpio_of_to_plat(struct udevice *dev)
{
	struct mchp_gpio_plat *plat = dev_get_plat(dev);

	plat->base = dev_read_addr_ptr(dev);
	if (!plat->base)
		return -EINVAL;

	return 0;
}

U_BOOT_DRIVER(gpio_mpfs) = {
	.name	= "gpio_mpfs",
	.id	= UCLASS_GPIO,
	.of_match = mchp_gpio_match,
	.of_to_plat = of_match_ptr(mchp_gpio_of_to_plat),
	.plat_auto	= sizeof(struct mchp_gpio_plat),
	.ops	= &mchp_gpio_ops,
	.probe	= mchp_gpio_probe,
};

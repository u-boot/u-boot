// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025-2026 RISCstar Ltd.
 */

#include <asm/gpio.h>
#include <clk.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <dm/read.h>
#include <dm/uclass.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/io.h>
#include <log.h>

#define GPIO_BANK_SIZE		32
#define GPIO_TO_BANK(pin)	((pin) / GPIO_BANK_SIZE)
#define GPIO_TO_BIT(pin)	((pin) % GPIO_BANK_SIZE)

#define to_spacemit_gpio_regs(priv, reg)	((priv)->data->offsets[reg])

enum spacemit_gpio_registers {
	SPACEMIT_GPLR,
	SPACEMIT_GPDR,
	SPACEMIT_GPSR,
	SPACEMIT_GPCR,
	SPACEMIT_GSDR,
	SPACEMIT_GCDR,
};

struct spacemit_gpio_data {
	u16	gpio_base;
	u16	gpio_count;
	u8	num_banks;
	const u16	*bank_offsets;
	const u16	*offsets;
};

struct spacemit_gpio_priv {
	void __iomem *regs;
	const struct spacemit_gpio_data *data;
};

static int spacemit_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			       struct ofnode_phandle_args *args)
{
	struct spacemit_gpio_data *data;
	u32 bank, offset, flags;

	data = (struct spacemit_gpio_data *)dev_get_driver_data(dev);
	if (args->args_count < 3) {
		dev_err(dev, "Invalid args count: %d, expected 3\n",
			args->args_count);
		return -EINVAL;
	}
	bank = args->args[0];
	offset = args->args[1];
	flags = args->args[2];

	if (bank >= data->num_banks) {
		dev_err(dev, "Invalid gpio bank: %u (max %u)\n",
			bank, data->num_banks - 1);
		return -EINVAL;
	}
	if (offset >= GPIO_BANK_SIZE) {
		dev_err(dev, "Invalid offset: %u (max 31)\n", offset);
		return -EINVAL;
	}
	desc->offset = bank * GPIO_BANK_SIZE + offset;
	desc->flags = gpio_flags_xlate(flags);
	return 0;
}

static int spacemit_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct spacemit_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *base, *addr;
	u32 value, mask;

	base = priv->regs + priv->data->bank_offsets[GPIO_TO_BANK(offset)];

	addr = base + to_spacemit_gpio_regs(priv, SPACEMIT_GPLR);
	value = readl(addr);
	mask = 1 << GPIO_TO_BIT(offset);
	return !!(value & mask);
}

static int spacemit_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct spacemit_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *base, *addr;
	u32 value, mask;

	base = priv->regs + priv->data->bank_offsets[GPIO_TO_BANK(offset)];

	addr = base + to_spacemit_gpio_regs(priv, SPACEMIT_GPDR);
	value = readl(addr);
	mask = 1 << GPIO_TO_BIT(offset);
	if (value & mask)
		return GPIOF_OUTPUT;
	return GPIOF_INPUT;
}

static int spacemit_gpio_get_flags(struct udevice *dev, unsigned int offset,
				   ulong *flagsp)
{
	ulong flags = 0;
	u32 dir;

	dir = spacemit_gpio_get_function(dev, offset);
	if (dir) {
		flags |= GPIOD_IS_OUT;
		if (spacemit_gpio_get_value(dev, offset))
			flags |= GPIOD_IS_OUT_ACTIVE;
	} else {
		flags |= GPIOD_IS_IN;
	}
	*flagsp = flags;
	return 0;
}

static int spacemit_gpio_set_flags(struct udevice *dev, unsigned int offset,
				   ulong flags)
{
	struct spacemit_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *base, *addr;
	int value;

	base = priv->regs + priv->data->bank_offsets[GPIO_TO_BANK(offset)];

	value = (flags & GPIOD_IS_OUT_ACTIVE) ? 1 : 0;
	if (flags & GPIOD_IS_IN) {
		addr = base + to_spacemit_gpio_regs(priv, SPACEMIT_GCDR);
		writel(1 << GPIO_TO_BIT(offset), addr);
	}
	if (flags & GPIOD_IS_OUT) {
		if (value) {
			addr = base + to_spacemit_gpio_regs(priv, SPACEMIT_GPSR);
			writel(1 << GPIO_TO_BIT(offset), addr);
		} else {
			addr = base + to_spacemit_gpio_regs(priv, SPACEMIT_GPCR);
			writel(1 << GPIO_TO_BIT(offset), addr);
		}
		addr = base + to_spacemit_gpio_regs(priv, SPACEMIT_GSDR);
		writel(1 << GPIO_TO_BIT(offset), addr);
	}
	return 0;
}

static int spacemit_gpio_request(struct udevice *dev, unsigned int offset,
				 const char *label)
{
	const struct pinctrl_ops *ops;
	struct udevice *pctldev;
	int ret;

	ret = uclass_first_device_err(UCLASS_PINCTRL, &pctldev);
	if (ret)
		return ret;

	ops = pinctrl_get_ops(pctldev);
	if (!ops->gpio_request_enable)
		return -ENOSYS;

	return ops->gpio_request_enable(pctldev, offset);
}

static int spacemit_gpio_rfree(struct udevice *dev, unsigned int offset)
{
	const struct pinctrl_ops *ops;
	struct udevice *pctldev;
	int ret;

	ret = uclass_first_device_err(UCLASS_PINCTRL, &pctldev);
	if (ret)
		return ret;

	ops = pinctrl_get_ops(pctldev);
	if (!ops->gpio_disable_free)
		return -ENOSYS;

	return ops->gpio_disable_free(pctldev, offset);
}

static const struct dm_gpio_ops spacemit_gpio_ops = {
	.request	= spacemit_gpio_request,
	.rfree		= spacemit_gpio_rfree,
	.xlate		= spacemit_gpio_xlate,
	.get_value	= spacemit_gpio_get_value,
	.get_function	= spacemit_gpio_get_function,
	.get_flags	= spacemit_gpio_get_flags,
	.set_flags	= spacemit_gpio_set_flags,
};

static int spacemit_gpio_probe(struct udevice *dev)
{
	struct spacemit_gpio_priv *priv;
	struct spacemit_gpio_data *data;
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct clk_bulk clks;
	int ret;

	data = (struct spacemit_gpio_data *)dev_get_driver_data(dev);
	priv = dev_get_priv(dev);
	priv->data = data;
	priv->regs = dev_read_addr_ptr(dev);
	if (!priv->regs) {
		dev_err(dev, "Fail to get base address\n");
		return -EINVAL;
	}
	uc_priv->bank_name = "GPIO";
	uc_priv->gpio_count = data->gpio_count;
	uc_priv->gpio_base = data->gpio_base;

	ret = clk_get_bulk(dev, &clks);
	if (ret) {
		dev_err(dev, "Fail to get bulk clks\n");
		return ret;
	}
	ret = clk_enable_bulk(&clks);
	if (ret) {
		dev_err(dev, "Fail to enable bulk clks\n");
		goto out;
	}
	return 0;
out:
	clk_release_bulk(&clks);
	return ret;
}

static const u16 spacemit_gpio_k1_offsets[] = {
	[SPACEMIT_GPLR] = 0x00,
	[SPACEMIT_GPDR] = 0x0c,
	[SPACEMIT_GPSR] = 0x18,
	[SPACEMIT_GPCR] = 0x24,
	[SPACEMIT_GSDR] = 0x54,
	[SPACEMIT_GCDR] = 0x60,
};

static const u16 spacemit_gpio_k1_bank_offsets[] = {
	0x0, 0x4, 0x8, 0x100,
};

static const u16 spacemit_gpio_k3_offsets[] = {
	[SPACEMIT_GPLR] = 0x00,
	[SPACEMIT_GPDR] = 0x04,
	[SPACEMIT_GPSR] = 0x08,
	[SPACEMIT_GPCR] = 0x0c,
	[SPACEMIT_GSDR] = 0x1c,
	[SPACEMIT_GCDR] = 0x20,
};

static const u16 spacemit_gpio_k3_bank_offsets[] = {
	0x0, 0x40, 0x80, 0x100,
};

static const struct spacemit_gpio_data k1_gpio_data = {
	.num_banks	= 4,
	.gpio_count	= 128,
	.gpio_base	= 0,
	.bank_offsets	= spacemit_gpio_k1_bank_offsets,
	.offsets	= spacemit_gpio_k1_offsets,
};

static const struct spacemit_gpio_data k3_gpio_data = {
	.num_banks	= 4,
	.gpio_count	= 128,
	.gpio_base	= 0,
	.bank_offsets	= spacemit_gpio_k3_bank_offsets,
	.offsets	= spacemit_gpio_k3_offsets,
};

static const struct udevice_id spacemit_gpio_ids[] = {
	{ .compatible = "spacemit,k1-gpio", .data = (uintptr_t)&k1_gpio_data, },
	{ .compatible = "spacemit,k3-gpio", .data = (uintptr_t)&k3_gpio_data, },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(k1_gpio) = {
	.name		= "spacemit_k1_gpio",
	.id		= UCLASS_GPIO,
	.of_match	= spacemit_gpio_ids,
	.ops		= &spacemit_gpio_ops,
	.priv_auto	= sizeof(struct spacemit_gpio_priv),
	.probe		= spacemit_gpio_probe,
};

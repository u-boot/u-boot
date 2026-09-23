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

static inline int gpio_to_reg_offset(unsigned int pin)
{
	unsigned int bank = GPIO_TO_BANK(pin);

	if (bank == 0)
		return 0;
	else if (bank == 1)
		return 4;
	else if (bank == 2)
		return 8;
	else if (bank == 3)
		return 0x100;
	log_warning("Use default GPIO bank for an invalid GPIO[%d].\n", pin);
	return 0;
}

#define REG_PLR(pin)		(0x00 + gpio_to_reg_offset(pin))
#define REG_PDR(pin)		(0x0c + gpio_to_reg_offset(pin))
#define REG_PSR(pin)		(0x18 + gpio_to_reg_offset(pin))
#define REG_PCR(pin)		(0x24 + gpio_to_reg_offset(pin))
#define REG_SDR(pin)		(0x54 + gpio_to_reg_offset(pin))
#define REG_CDR(pin)		(0x60 + gpio_to_reg_offset(pin))

struct spacemit_gpio_data {
	u16	gpio_base;
	u16	gpio_count;
	u8	num_banks;
};

struct spacemit_gpio_priv {
	void __iomem *regs;
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
	void __iomem *addr;
	u32 value, mask;

	addr = priv->regs + REG_PLR(offset);
	value = readl(addr);
	mask = 1 << GPIO_TO_BIT(offset);
	return !!(value & mask);
}

static int spacemit_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct spacemit_gpio_priv *priv = dev_get_priv(dev);
	void __iomem *addr;
	u32 value, mask;

	addr = priv->regs + REG_PDR(offset);
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
	void __iomem *addr;
	int value;

	value = (flags & GPIOD_IS_OUT_ACTIVE) ? 1 : 0;
	if (flags & GPIOD_IS_IN) {
		addr = priv->regs + REG_CDR(offset);
		writel(1 << GPIO_TO_BIT(offset), addr);
	}
	if (flags & GPIOD_IS_OUT) {
		if (value) {
			addr = priv->regs + REG_PSR(offset);
			writel(1 << GPIO_TO_BIT(offset), addr);
		} else {
			addr = priv->regs + REG_PCR(offset);
			writel(1 << GPIO_TO_BIT(offset), addr);
		}
		addr = priv->regs + REG_SDR(offset);
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

static const struct spacemit_gpio_data k1_gpio_data = {
	.num_banks	= 4,
	.gpio_count	= 128,
	.gpio_base	= 0,
};

static const struct udevice_id spacemit_gpio_ids[] = {
	{ .compatible = "spacemit,k1-gpio", .data = (uintptr_t)&k1_gpio_data, },
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

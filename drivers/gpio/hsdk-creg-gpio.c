/*
 * Synopsys HSDK SDP Generic PLL clock driver
 *
 * Copyright (C) 2017 Synopsys
 * Author: Eugeniy Paltsev <Eugeniy.Paltsev@synopsys.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <asm-generic/gpio.h>
#include <asm/io.h>
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <linux/printk.h>

#define HSDK_CREG_MAX_GPIO	8

#define GPIO_ACTIVATE		0x2
#define GPIO_DEACTIVATE		0x3
#define GPIO_PIN_MASK		0x3
#define BIT_PER_GPIO		2

struct hsdk_creg_gpio {
	uint32_t *regs;
};

static int hsdk_creg_gpio_set_value(struct udevice *dev, unsigned oft, int val)
{
	struct hsdk_creg_gpio *hcg = dev_get_priv(dev);
	uint32_t reg = readl(hcg->regs);
	uint32_t cmd = val ? GPIO_DEACTIVATE : GPIO_ACTIVATE;

	reg &= ~(GPIO_PIN_MASK << (oft * BIT_PER_GPIO));
	reg |=  (cmd << (oft * BIT_PER_GPIO));

	writel(reg, hcg->regs);

	return 0;
}

static int hsdk_creg_gpio_direction_output(struct udevice *dev, unsigned oft,
					   int val)
{
	hsdk_creg_gpio_set_value(dev, oft, val);

	return 0;
}

static int hsdk_creg_gpio_direction_input(struct udevice *dev, unsigned oft)
{
	pr_err("hsdk-creg-gpio can't be used as input!\n");

	return -ENOTSUPP;
}

static int hsdk_creg_gpio_get_value(struct udevice *dev, unsigned int oft)
{
	struct hsdk_creg_gpio *hcg = dev_get_priv(dev);
	uint32_t val = readl(hcg->regs);

	val = (val >> (oft * BIT_PER_GPIO)) & GPIO_PIN_MASK;
	return (val == GPIO_DEACTIVATE) ? 1 : 0;
}

static const struct dm_gpio_ops hsdk_creg_gpio_ops = {
	.direction_output	= hsdk_creg_gpio_direction_output,
	.direction_input	= hsdk_creg_gpio_direction_input,
	.set_value		= hsdk_creg_gpio_set_value,
	.get_value		= hsdk_creg_gpio_get_value,
};

static int hsdk_creg_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct hsdk_creg_gpio *hcg = dev_get_priv(dev);

	hcg->regs = (uint32_t *)devfdt_get_addr_ptr(dev);

	uc_priv->gpio_count = dev_read_u32_default(dev, "gpio-count", 1);
	if (uc_priv->gpio_count > HSDK_CREG_MAX_GPIO)
		uc_priv->gpio_count = HSDK_CREG_MAX_GPIO;

	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");
	if (!uc_priv->bank_name)
		uc_priv->bank_name = dev_read_name(dev);

	pr_debug("%s GPIO [0x%p] controller with %d gpios probed\n",
		 uc_priv->bank_name, hcg->regs, uc_priv->gpio_count);

	return 0;
}

static const struct udevice_id hsdk_creg_gpio_ids[] = {
	{ .compatible = "snps,hsdk-creg-gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_hsdk_creg) = {
	.name	= "gpio_hsdk_creg",
	.id	= UCLASS_GPIO,
	.ops	= &hsdk_creg_gpio_ops,
	.probe	= hsdk_creg_gpio_probe,
	.of_match = hsdk_creg_gpio_ids,
	.platdata_auto_alloc_size = sizeof(struct hsdk_creg_gpio),
};

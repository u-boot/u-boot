/*
 * (C) Copyright 2015 Google, Inc
 *
 * (C) Copyright 2008-2014 Rockchip Electronics
 * Peter, Software Engineering, <superpeter.cai@gmail.com>.
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <syscon.h>
#include <asm/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/clock/rk3288-cru.h>

enum {
	ROCKCHIP_GPIOS_PER_BANK		= 32,
};

#define OFFSET_TO_BIT(bit)	(1UL << (bit))

struct rockchip_gpio_priv {
	struct rockchip_gpio_regs *regs;
	struct udevice *pinctrl;
	int bank;
	char name[2];
};

static int rockchip_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;

	clrbits_le32(&regs->swport_ddr, OFFSET_TO_BIT(offset));

	return 0;
}

static int rockchip_gpio_direction_output(struct udevice *dev, unsigned offset,
					  int value)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;
	int mask = OFFSET_TO_BIT(offset);

	clrsetbits_le32(&regs->swport_dr, mask, value ? mask : 0);
	setbits_le32(&regs->swport_ddr, mask);

	return 0;
}

static int rockchip_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;

	return readl(&regs->ext_port) & OFFSET_TO_BIT(offset) ? 1 : 0;
}

static int rockchip_gpio_set_value(struct udevice *dev, unsigned offset,
				   int value)
{
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;
	int mask = OFFSET_TO_BIT(offset);

	clrsetbits_le32(&regs->swport_dr, mask, value ? mask : 0);

	return 0;
}

static int rockchip_gpio_get_function(struct udevice *dev, unsigned offset)
{
#ifdef CONFIG_SPL_BUILD
	return -ENODATA;
#else
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	struct rockchip_gpio_regs *regs = priv->regs;
	bool is_output;
	int ret;

	ret = pinctrl_get_gpio_mux(priv->pinctrl, priv->bank, offset);
	if (ret)
		return ret;

	/* If it's not 0, then it is not a GPIO */
	if (ret)
		return GPIOF_FUNC;
	is_output = readl(&regs->swport_ddr) & OFFSET_TO_BIT(offset);

	return is_output ? GPIOF_OUTPUT : GPIOF_INPUT;
#endif
}

static int rockchip_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			    struct fdtdec_phandle_args *args)
{
	desc->offset = args->args[0];
	desc->flags = args->args[1] & GPIO_ACTIVE_LOW ? GPIOD_ACTIVE_LOW : 0;

	return 0;
}

static int rockchip_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct rockchip_gpio_priv *priv = dev_get_priv(dev);
	char *end;
	int ret;

	/* This only supports RK3288 at present */
	priv->regs = (struct rockchip_gpio_regs *)dev_get_addr(dev);
	ret = uclass_first_device_err(UCLASS_PINCTRL, &priv->pinctrl);
	if (ret)
		return ret;

	uc_priv->gpio_count = ROCKCHIP_GPIOS_PER_BANK;
	end = strrchr(dev->name, '@');
	priv->bank = trailing_strtoln(dev->name, end);
	priv->name[0] = 'A' + priv->bank;
	uc_priv->bank_name = priv->name;

	return 0;
}

static const struct dm_gpio_ops gpio_rockchip_ops = {
	.direction_input	= rockchip_gpio_direction_input,
	.direction_output	= rockchip_gpio_direction_output,
	.get_value		= rockchip_gpio_get_value,
	.set_value		= rockchip_gpio_set_value,
	.get_function		= rockchip_gpio_get_function,
	.xlate			= rockchip_gpio_xlate,
};

static const struct udevice_id rockchip_gpio_ids[] = {
	{ .compatible = "rockchip,gpio-bank" },
	{ }
};

U_BOOT_DRIVER(gpio_rockchip) = {
	.name	= "gpio_rockchip",
	.id	= UCLASS_GPIO,
	.of_match = rockchip_gpio_ids,
	.ops	= &gpio_rockchip_ops,
	.priv_auto_alloc_size = sizeof(struct rockchip_gpio_priv),
	.probe	= rockchip_gpio_probe,
};

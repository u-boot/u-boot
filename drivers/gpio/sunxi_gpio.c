// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * Based on earlier arch/arm/cpu/armv7/sunxi/gpio.c:
 *
 * (C) Copyright 2007-2011
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Tom Cubie <tangliang@allwinnertech.com>
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <dt-bindings/gpio/gpio.h>

#if !CONFIG_IS_ENABLED(DM_GPIO)
static int sunxi_gpio_output(u32 pin, u32 val)
{
	u32 dat;
	u32 bank = GPIO_BANK(pin);
	u32 num = GPIO_NUM(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	dat = readl(&pio->dat);
	if (val)
		dat |= 0x1 << num;
	else
		dat &= ~(0x1 << num);

	writel(dat, &pio->dat);

	return 0;
}

static int sunxi_gpio_input(u32 pin)
{
	u32 dat;
	u32 bank = GPIO_BANK(pin);
	u32 num = GPIO_NUM(pin);
	struct sunxi_gpio *pio = BANK_TO_GPIO(bank);

	dat = readl(&pio->dat);
	dat >>= num;

	return dat & 0x1;
}

int gpio_request(unsigned gpio, const char *label)
{
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	sunxi_gpio_set_cfgpin(gpio, SUNXI_GPIO_INPUT);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	sunxi_gpio_set_cfgpin(gpio, SUNXI_GPIO_OUTPUT);

	return sunxi_gpio_output(gpio, value);
}

int gpio_get_value(unsigned gpio)
{
	return sunxi_gpio_input(gpio);
}

int gpio_set_value(unsigned gpio, int value)
{
	return sunxi_gpio_output(gpio, value);
}

int sunxi_name_to_gpio(const char *name)
{
	int group = 0;
	int groupsize = 9 * 32;
	long pin;
	char *eptr;

	if (*name == 'P' || *name == 'p')
		name++;
	if (*name >= 'A') {
		group = *name - (*name > 'a' ? 'a' : 'A');
		groupsize = 32;
		name++;
	}

	pin = simple_strtol(name, &eptr, 10);
	if (!*name || *eptr)
		return -1;
	if (pin < 0 || pin > groupsize || group >= 9)
		return -1;
	return group * 32 + pin;
}
#endif /* DM_GPIO */

#if CONFIG_IS_ENABLED(DM_GPIO)
/* TODO(sjg@chromium.org): Remove this function and use device tree */
int sunxi_name_to_gpio(const char *name)
{
	unsigned int gpio;
	int ret;
#if !defined CONFIG_SPL_BUILD && defined CONFIG_AXP_GPIO
	char lookup[8];

	if (strcasecmp(name, "AXP0-VBUS-DETECT") == 0) {
		sprintf(lookup, SUNXI_GPIO_AXP0_PREFIX "%d",
			SUNXI_GPIO_AXP0_VBUS_DETECT);
		name = lookup;
	} else if (strcasecmp(name, "AXP0-VBUS-ENABLE") == 0) {
		sprintf(lookup, SUNXI_GPIO_AXP0_PREFIX "%d",
			SUNXI_GPIO_AXP0_VBUS_ENABLE);
		name = lookup;
	}
#endif
	ret = gpio_lookup_name(name, NULL, NULL, &gpio);

	return ret ? ret : gpio;
}

static int sunxi_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct sunxi_gpio_plat *plat = dev_get_plat(dev);
	u32 num = GPIO_NUM(offset);
	unsigned dat;

	dat = readl(&plat->regs->dat);
	dat >>= num;

	return dat & 0x1;
}

static int sunxi_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct sunxi_gpio_plat *plat = dev_get_plat(dev);
	int func;

	func = sunxi_gpio_get_cfgbank(plat->regs, offset);
	if (func == SUNXI_GPIO_OUTPUT)
		return GPIOF_OUTPUT;
	else if (func == SUNXI_GPIO_INPUT)
		return GPIOF_INPUT;
	else
		return GPIOF_FUNC;
}

static int sunxi_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			    struct ofnode_phandle_args *args)
{
	int ret;

	ret = device_get_child(dev, args->args[0], &desc->dev);
	if (ret)
		return ret;
	desc->offset = args->args[1];
	desc->flags = gpio_flags_xlate(args->args[2]);

	return 0;
}

static int sunxi_gpio_set_flags(struct udevice *dev, unsigned int offset,
				ulong flags)
{
	struct sunxi_gpio_plat *plat = dev_get_plat(dev);

	if (flags & GPIOD_IS_OUT) {
		u32 value = !!(flags & GPIOD_IS_OUT_ACTIVE);
		u32 num = GPIO_NUM(offset);

		clrsetbits_le32(&plat->regs->dat, 1 << num, value << num);
		sunxi_gpio_set_cfgbank(plat->regs, offset, SUNXI_GPIO_OUTPUT);
	} else if (flags & GPIOD_IS_IN) {
		u32 pull = 0;

		if (flags & GPIOD_PULL_UP)
			pull = 1;
		else if (flags & GPIOD_PULL_DOWN)
			pull = 2;
		sunxi_gpio_set_pull_bank(plat->regs, offset, pull);
		sunxi_gpio_set_cfgbank(plat->regs, offset, SUNXI_GPIO_INPUT);
	}

	return 0;
}

static const struct dm_gpio_ops gpio_sunxi_ops = {
	.get_value		= sunxi_gpio_get_value,
	.get_function		= sunxi_gpio_get_function,
	.xlate			= sunxi_gpio_xlate,
	.set_flags		= sunxi_gpio_set_flags,
};

static int gpio_sunxi_probe(struct udevice *dev)
{
	struct sunxi_gpio_plat *plat = dev_get_plat(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* Tell the uclass how many GPIOs we have */
	if (plat) {
		uc_priv->gpio_count = SUNXI_GPIOS_PER_BANK;
		uc_priv->bank_name = plat->bank_name;
	}

	return 0;
}

U_BOOT_DRIVER(gpio_sunxi) = {
	.name	= "gpio_sunxi",
	.id	= UCLASS_GPIO,
	.probe	= gpio_sunxi_probe,
	.ops	= &gpio_sunxi_ops,
};
#endif /* DM_GPIO */

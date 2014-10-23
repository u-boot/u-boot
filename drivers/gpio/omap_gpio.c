/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * SPDX-License-Identifier:	GPL-2.0
 *
 * This work is derived from the linux 2.6.27 kernel source
 * To fetch, use the kernel repository
 * git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
 * Use the v2.6.27 tag.
 *
 * Below is the original's header including its copyright
 *
 *  linux/arch/arm/plat-omap/gpio.c
 *
 * Support functions for OMAP GPIO
 *
 * Copyright (C) 2003-2005 Nokia Corporation
 * Written by Juha Yrjölä <juha.yrjola@nokia.com>
 */
#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/errno.h>

#define OMAP_GPIO_DIR_OUT	0
#define OMAP_GPIO_DIR_IN	1

#ifdef CONFIG_DM_GPIO

#define GPIO_NAME_SIZE			20
#define GPIO_PER_BANK			32

struct gpio_bank {
	char label[GPIO_PER_BANK][GPIO_NAME_SIZE];
	/* TODO(sjg@chromium.org): Can we use a struct here? */
	void *base;	/* address of registers in physical memory */
	enum gpio_method method;
};

#endif

static inline int get_gpio_index(int gpio)
{
	return gpio & 0x1f;
}

int gpio_is_valid(int gpio)
{
	return (gpio >= 0) && (gpio < OMAP_MAX_GPIO);
}

static void _set_gpio_direction(const struct gpio_bank *bank, int gpio,
				int is_input)
{
	void *reg = bank->base;
	u32 l;

	switch (bank->method) {
	case METHOD_GPIO_24XX:
		reg += OMAP_GPIO_OE;
		break;
	default:
		return;
	}
	l = __raw_readl(reg);
	if (is_input)
		l |= 1 << gpio;
	else
		l &= ~(1 << gpio);
	__raw_writel(l, reg);
}

/**
 * Get the direction of the GPIO by reading the GPIO_OE register
 * corresponding to the specified bank.
 */
static int _get_gpio_direction(const struct gpio_bank *bank, int gpio)
{
	void *reg = bank->base;
	u32 v;

	switch (bank->method) {
	case METHOD_GPIO_24XX:
		reg += OMAP_GPIO_OE;
		break;
	default:
		return -1;
	}

	v = __raw_readl(reg);

	if (v & (1 << gpio))
		return OMAP_GPIO_DIR_IN;
	else
		return OMAP_GPIO_DIR_OUT;
}

static void _set_gpio_dataout(const struct gpio_bank *bank, int gpio,
				int enable)
{
	void *reg = bank->base;
	u32 l = 0;

	switch (bank->method) {
	case METHOD_GPIO_24XX:
		if (enable)
			reg += OMAP_GPIO_SETDATAOUT;
		else
			reg += OMAP_GPIO_CLEARDATAOUT;
		l = 1 << gpio;
		break;
	default:
		printf("omap3-gpio unknown bank method %s %d\n",
		       __FILE__, __LINE__);
		return;
	}
	__raw_writel(l, reg);
}

static int _get_gpio_value(const struct gpio_bank *bank, int gpio)
{
	void *reg = bank->base;
	int input;

	switch (bank->method) {
	case METHOD_GPIO_24XX:
		input = _get_gpio_direction(bank, gpio);
		switch (input) {
		case OMAP_GPIO_DIR_IN:
			reg += OMAP_GPIO_DATAIN;
			break;
		case OMAP_GPIO_DIR_OUT:
			reg += OMAP_GPIO_DATAOUT;
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
	}

	return (__raw_readl(reg) & (1 << gpio)) != 0;
}

#ifndef CONFIG_DM_GPIO

static inline const struct gpio_bank *get_gpio_bank(int gpio)
{
	return &omap_gpio_bank[gpio >> 5];
}

static int check_gpio(int gpio)
{
	if (!gpio_is_valid(gpio)) {
		printf("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
}

/**
 * Set value of the specified gpio
 */
int gpio_set_value(unsigned gpio, int value)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;
	bank = get_gpio_bank(gpio);
	_set_gpio_dataout(bank, get_gpio_index(gpio), value);

	return 0;
}

/**
 * Get value of the specified gpio
 */
int gpio_get_value(unsigned gpio)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;
	bank = get_gpio_bank(gpio);

	return _get_gpio_value(bank, get_gpio_index(gpio));
}

/**
 * Set gpio direction as input
 */
int gpio_direction_input(unsigned gpio)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;

	bank = get_gpio_bank(gpio);
	_set_gpio_direction(bank, get_gpio_index(gpio), 1);

	return 0;
}

/**
 * Set gpio direction as output
 */
int gpio_direction_output(unsigned gpio, int value)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return -1;

	bank = get_gpio_bank(gpio);
	_set_gpio_dataout(bank, get_gpio_index(gpio), value);
	_set_gpio_direction(bank, get_gpio_index(gpio), 0);

	return 0;
}

/**
 * Request a gpio before using it.
 *
 * NOTE: Argument 'label' is unused.
 */
int gpio_request(unsigned gpio, const char *label)
{
	if (check_gpio(gpio) < 0)
		return -1;

	return 0;
}

/**
 * Reset and free the gpio after using it.
 */
int gpio_free(unsigned gpio)
{
	return 0;
}

#else /* new driver model interface CONFIG_DM_GPIO */

/**
 * gpio_is_requested() - check if a GPIO has been requested
 *
 * @bank:	Bank to check
 * @offset:	GPIO offset within bank to check
 * @return true if marked as requested, false if not
 */
static inline bool gpio_is_requested(struct gpio_bank *bank, int offset)
{
	return *bank->label[offset] != '\0';
}

static int omap_gpio_is_output(struct gpio_bank *bank, int offset)
{
	return _get_gpio_direction(bank, offset) == OMAP_GPIO_DIR_OUT;
}

static int check_requested(struct udevice *dev, unsigned offset,
			   const char *func)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

	if (!gpio_is_requested(bank, offset)) {
		printf("omap_gpio: %s: error: gpio %s%d not requested\n",
		       func, uc_priv->bank_name, offset);
		return -EPERM;
	}

	return 0;
}

/* set GPIO pin 'gpio' as an input */
static int omap_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	/* Configure GPIO direction as input. */
	_set_gpio_direction(bank, offset, 1);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
static int omap_gpio_direction_output(struct udevice *dev, unsigned offset,
				       int value)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	_set_gpio_dataout(bank, offset, value);
	_set_gpio_direction(bank, offset, 0);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
static int omap_gpio_get_value(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	return _get_gpio_value(bank, offset);
}

/* write GPIO OUT value to pin 'gpio' */
static int omap_gpio_set_value(struct udevice *dev, unsigned offset,
				 int value)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;

	_set_gpio_dataout(bank, offset, value);

	return 0;
}

static int omap_gpio_get_state(struct udevice *dev, unsigned int offset,
			      char *buf, int bufsize)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct gpio_bank *bank = dev_get_priv(dev);
	const char *label;
	bool requested;
	bool is_output;
	int size;

	label = bank->label[offset];
	is_output = omap_gpio_is_output(bank->base, offset);
	size = snprintf(buf, bufsize, "%s%d: ",
			uc_priv->bank_name ? uc_priv->bank_name : "", offset);
	buf += size;
	bufsize -= size;
	requested = gpio_is_requested(bank, offset);
	snprintf(buf, bufsize, "%s: %d [%c]%s%s",
		 is_output ? "out" : " in",
		 _get_gpio_value(bank, offset),
		 requested ? 'x' : ' ',
		 requested ? " " : "",
		 label);

	return 0;
}

static int omap_gpio_request(struct udevice *dev, unsigned offset,
			      const char *label)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	if (gpio_is_requested(bank, offset))
		return -EBUSY;

	strncpy(bank->label[offset], label, GPIO_NAME_SIZE);
	bank->label[offset][GPIO_NAME_SIZE - 1] = '\0';

	return 0;
}

static int omap_gpio_free(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	int ret;

	ret = check_requested(dev, offset, __func__);
	if (ret)
		return ret;
	bank->label[offset][0] = '\0';

	return 0;
}

static int omap_gpio_get_function(struct udevice *dev, unsigned offset)
{
	struct gpio_bank *bank = dev_get_priv(dev);

	if (!gpio_is_requested(bank, offset))
		return GPIOF_UNUSED;

	/* GPIOF_FUNC is not implemented yet */
	if (_get_gpio_direction(bank->base, offset))
		return GPIOF_OUTPUT;
	else
		return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_omap_ops = {
	.request		= omap_gpio_request,
	.free			= omap_gpio_free,
	.direction_input	= omap_gpio_direction_input,
	.direction_output	= omap_gpio_direction_output,
	.get_value		= omap_gpio_get_value,
	.set_value		= omap_gpio_set_value,
	.get_function		= omap_gpio_get_function,
	.get_state		= omap_gpio_get_state,
};

static int omap_gpio_probe(struct udevice *dev)
{
	struct gpio_bank *bank = dev_get_priv(dev);
	struct omap_gpio_platdata *plat = dev_get_platdata(dev);
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	char name[18], *str;

	sprintf(name, "GPIO%d_", plat->bank_index);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	uc_priv->bank_name = str;
	uc_priv->gpio_count = GPIO_PER_BANK;
	bank->base = (void *)plat->base;
	bank->method = plat->method;

	return 0;
}

U_BOOT_DRIVER(gpio_omap) = {
	.name	= "gpio_omap",
	.id	= UCLASS_GPIO,
	.ops	= &gpio_omap_ops,
	.probe	= omap_gpio_probe,
	.priv_auto_alloc_size = sizeof(struct gpio_bank),
};

#endif /* CONFIG_DM_GPIO */

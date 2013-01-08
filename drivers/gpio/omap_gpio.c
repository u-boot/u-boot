/*
 * Copyright (c) 2009 Wind River Systems, Inc.
 * Tom Rix <Tom.Rix@windriver.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/errno.h>

#define OMAP_GPIO_DIR_OUT	0
#define OMAP_GPIO_DIR_IN	1

static inline const struct gpio_bank *get_gpio_bank(int gpio)
{
	return &omap_gpio_bank[gpio >> 5];
}

static inline int get_gpio_index(int gpio)
{
	return gpio & 0x1f;
}

int gpio_is_valid(int gpio)
{
	return (gpio >= 0) && (gpio < 192);
}

static int check_gpio(int gpio)
{
	if (!gpio_is_valid(gpio)) {
		printf("ERROR : check_gpio: invalid GPIO %d\n", gpio);
		return -1;
	}
	return 0;
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
	void *reg;
	int input;

	if (check_gpio(gpio) < 0)
		return -1;
	bank = get_gpio_bank(gpio);
	reg = bank->base;
	switch (bank->method) {
	case METHOD_GPIO_24XX:
		input = _get_gpio_direction(bank, get_gpio_index(gpio));
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
	return (__raw_readl(reg)
			& (1 << get_gpio_index(gpio))) != 0;
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

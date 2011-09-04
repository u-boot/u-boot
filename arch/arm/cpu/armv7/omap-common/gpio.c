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
#include <asm/arch/gpio.h>
#include <asm/io.h>
#include <asm/errno.h>

static inline const struct gpio_bank *get_gpio_bank(int gpio)
{
	return &omap_gpio_bank[gpio >> 5];
}

static inline int get_gpio_index(int gpio)
{
	return gpio & 0x1f;
}

static inline int gpio_valid(int gpio)
{
	if (gpio < 0)
		return -1;
	if (gpio < 192)
		return 0;
	return -1;
}

static int check_gpio(int gpio)
{
	if (gpio_valid(gpio) < 0) {
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

void omap_set_gpio_direction(int gpio, int is_input)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return;
	bank = get_gpio_bank(gpio);
	_set_gpio_direction(bank, get_gpio_index(gpio), is_input);
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

void omap_set_gpio_dataout(int gpio, int enable)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return;
	bank = get_gpio_bank(gpio);
	_set_gpio_dataout(bank, get_gpio_index(gpio), enable);
}

int omap_get_gpio_datain(int gpio)
{
	const struct gpio_bank *bank;
	void *reg;

	if (check_gpio(gpio) < 0)
		return -EINVAL;
	bank = get_gpio_bank(gpio);
	reg = bank->base;
	switch (bank->method) {
	case METHOD_GPIO_24XX:
		reg += OMAP_GPIO_DATAIN;
		break;
	default:
		return -EINVAL;
	}
	return (__raw_readl(reg)
			& (1 << get_gpio_index(gpio))) != 0;
}

static void _reset_gpio(const struct gpio_bank *bank, int gpio)
{
	_set_gpio_direction(bank, get_gpio_index(gpio), 1);
}

int omap_request_gpio(int gpio)
{
	if (check_gpio(gpio) < 0)
		return -EINVAL;

	return 0;
}

void omap_free_gpio(int gpio)
{
	const struct gpio_bank *bank;

	if (check_gpio(gpio) < 0)
		return;
	bank = get_gpio_bank(gpio);

	_reset_gpio(bank, gpio);
}

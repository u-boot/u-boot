/*
 * Copyright (C) 2012 Vikram Narayananan
 * <vikram186@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>

inline int gpio_is_valid(unsigned gpio)
{
	return (gpio < BCM2835_GPIO_COUNT);
}

int gpio_request(unsigned gpio, const char *label)
{
	return !gpio_is_valid(gpio);
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	struct bcm2835_gpio_regs *reg =
		(struct bcm2835_gpio_regs *)BCM2835_GPIO_BASE;
	unsigned val;

	val = readl(&reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio));
	val |= (BCM2835_GPIO_INPUT << BCM2835_GPIO_FSEL_SHIFT(gpio));
	writel(val, &reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	struct bcm2835_gpio_regs *reg =
		(struct bcm2835_gpio_regs *)BCM2835_GPIO_BASE;
	unsigned val;

	gpio_set_value(gpio, value);

	val = readl(&reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);
	val &= ~(BCM2835_GPIO_FSEL_MASK << BCM2835_GPIO_FSEL_SHIFT(gpio));
	val |= (BCM2835_GPIO_OUTPUT << BCM2835_GPIO_FSEL_SHIFT(gpio));
	writel(val, &reg->gpfsel[BCM2835_GPIO_FSEL_BANK(gpio)]);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	struct bcm2835_gpio_regs *reg =
		(struct bcm2835_gpio_regs *)BCM2835_GPIO_BASE;
	unsigned val;

	val = readl(&reg->gplev[BCM2835_GPIO_COMMON_BANK(gpio)]);

	return (val >> BCM2835_GPIO_COMMON_SHIFT(gpio)) & 0x1;
}

int gpio_set_value(unsigned gpio, int value)
{
	struct bcm2835_gpio_regs *reg =
		(struct bcm2835_gpio_regs *)BCM2835_GPIO_BASE;
	u32 *output_reg = value ? reg->gpset : reg->gpclr;

	writel(1 << BCM2835_GPIO_COMMON_SHIFT(gpio),
				&output_reg[BCM2835_GPIO_COMMON_BANK(gpio)]);

	return 0;
}

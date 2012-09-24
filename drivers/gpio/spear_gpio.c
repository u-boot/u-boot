/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Driver for SPEAr600 GPIO controller
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <errno.h>

static int gpio_direction(unsigned gpio,
			  enum gpio_direction direction)
{
	struct gpio_regs *regs = (struct gpio_regs *)CONFIG_GPIO_BASE;
	u32 val;

	val = readl(&regs->gpiodir);

	if (direction == GPIO_DIRECTION_OUT)
		val |= 1 << gpio;
	else
		val &= ~(1 << gpio);

	writel(val, &regs->gpiodir);

	return 0;
}

int gpio_set_value(unsigned gpio, int value)
{
	struct gpio_regs *regs = (struct gpio_regs *)CONFIG_GPIO_BASE;

	writel(1 << gpio, &regs->gpiodata[DATA_REG_ADDR(gpio)]);

	return 0;
}

int gpio_get_value(unsigned gpio)
{
	struct gpio_regs *regs = (struct gpio_regs *)CONFIG_GPIO_BASE;
	u32 val;

	val = readl(&regs->gpiodata[DATA_REG_ADDR(gpio)]);

	return !!val;
}

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= SPEAR_GPIO_COUNT)
		return -EINVAL;

	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

void gpio_toggle_value(unsigned gpio)
{
	gpio_set_value(gpio, !gpio_get_value(gpio));
}

int gpio_direction_input(unsigned gpio)
{
	return gpio_direction(gpio, GPIO_DIRECTION_IN);
}

int gpio_direction_output(unsigned gpio, int value)
{
	int ret = gpio_direction(gpio, GPIO_DIRECTION_OUT);

	if (ret < 0)
		return ret;

	gpio_set_value(gpio, value);
	return 0;
}

/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include "mvgpio.h"
#include <asm/gpio.h>

#ifndef MV_MAX_GPIO
#define MV_MAX_GPIO	128
#endif

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO requested %d\n", __func__, gpio);
		return -1;
	}
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}

int gpio_direction_input(unsigned gpio)
{
	struct gpio_reg *gpio_reg_bank;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gcdr);
	return 0;
}

int gpio_direction_output(unsigned gpio, int value)
{
	struct gpio_reg *gpio_reg_bank;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gsdr);
	gpio_set_value(gpio, value);
	return 0;
}

int gpio_get_value(unsigned gpio)
{
	struct gpio_reg *gpio_reg_bank;
	u32 gpio_val;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	gpio_val = readl(&gpio_reg_bank->gplr);

	return GPIO_VAL(gpio, gpio_val);
}

int gpio_set_value(unsigned gpio, int value)
{
	struct gpio_reg *gpio_reg_bank;

	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gpio);
		return -1;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gpio));
	if (value)
		writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gpsr);
	else
		writel(GPIO_TO_BIT(gpio), &gpio_reg_bank->gpcr);

	return 0;
}

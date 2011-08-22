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

int gpio_request(int gp, const char *label)
{
	if (gp >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO requested %d\n", __func__, gp);
		return -EINVAL;
	}
	return 0;
}

void gpio_free(int gp)
{
}

void gpio_toggle_value(int gp)
{
	gpio_set_value(gp, !gpio_get_value(gp));
}

int gpio_direction_input(int gp)
{
	struct gpio_reg *gpio_reg_bank;

	if (gp >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gp);
		return -EINVAL;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gp));
	writel(GPIO_TO_BIT(gp), &gpio_reg_bank->gcdr);
	return 0;
}

int gpio_direction_output(int gp, int value)
{
	struct gpio_reg *gpio_reg_bank;

	if (gp >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gp);
		return -EINVAL;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gp));
	writel(GPIO_TO_BIT(gp), &gpio_reg_bank->gsdr);
	gpio_set_value(gp, value);
	return 0;
}

int gpio_get_value(int gp)
{
	struct gpio_reg *gpio_reg_bank;
	u32 gp_val;

	if (gp >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gp);
		return -EINVAL;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gp));
	gp_val = readl(&gpio_reg_bank->gplr);

	return GPIO_VAL(gp, gp_val);
}

void gpio_set_value(int gp, int value)
{
	struct gpio_reg *gpio_reg_bank;

	if (gp >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO %d\n", __func__, gp);
		return;
	}

	gpio_reg_bank = get_gpio_base(GPIO_TO_REG(gp));
	if (value)
		writel(GPIO_TO_BIT(gp),	&gpio_reg_bank->gpsr);
	else
		writel(GPIO_TO_BIT(gp),	&gpio_reg_bank->gpcr);
}

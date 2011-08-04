/*
 * GPIO driver for TI DaVinci DA8xx SOCs.
 *
 * (C) Copyright 2011 Guralp Systems Ltd.
 * Laurence Withers <lwithers@guralp.com>
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

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/gpio.h>
#include <asm/arch/hardware.h>
#include <asm/arch/davinci_misc.h>


static struct gpio_registry {
	int is_registered;
	char name[GPIO_NAME_SIZE];
} gpio_registry[MAX_NUM_GPIOS];


#define pinmux(x)       (&davinci_syscfg_regs->pinmux[x])

static const struct pinmux_config gpio_pinmux[] = {
	{ pinmux(1), 8, 7 },	/* GP0[0] */
	{ pinmux(1), 8, 6 },
	{ pinmux(1), 8, 5 },
	{ pinmux(1), 8, 4 },
	{ pinmux(1), 8, 3 },
	{ pinmux(1), 8, 2 },
	{ pinmux(1), 8, 1 },
	{ pinmux(1), 8, 0 },
	{ pinmux(0), 8, 7 },
	{ pinmux(0), 8, 6 },
	{ pinmux(0), 8, 5 },
	{ pinmux(0), 8, 4 },
	{ pinmux(0), 8, 3 },
	{ pinmux(0), 8, 2 },
	{ pinmux(0), 8, 1 },
	{ pinmux(0), 8, 0 },
	{ pinmux(4), 8, 7 },	/* GP1[0] */
	{ pinmux(4), 8, 6 },
	{ pinmux(4), 8, 5 },
	{ pinmux(4), 8, 4 },
	{ pinmux(4), 8, 3 },
	{ pinmux(4), 8, 2 },
	{ pinmux(4), 4, 1 },
	{ pinmux(4), 4, 0 },
	{ pinmux(3), 4, 0 },
	{ pinmux(2), 4, 6 },
	{ pinmux(2), 4, 5 },
	{ pinmux(2), 4, 4 },
	{ pinmux(2), 4, 3 },
	{ pinmux(2), 4, 2 },
	{ pinmux(2), 4, 1 },
	{ pinmux(2), 8, 0 },
	{ pinmux(6), 8, 7 },	/* GP2[0] */
	{ pinmux(6), 8, 6 },
	{ pinmux(6), 8, 5 },
	{ pinmux(6), 8, 4 },
	{ pinmux(6), 8, 3 },
	{ pinmux(6), 8, 2 },
	{ pinmux(6), 8, 1 },
	{ pinmux(6), 8, 0 },
	{ pinmux(5), 8, 7 },
	{ pinmux(5), 8, 6 },
	{ pinmux(5), 8, 5 },
	{ pinmux(5), 8, 4 },
	{ pinmux(5), 8, 3 },
	{ pinmux(5), 8, 2 },
	{ pinmux(5), 8, 1 },
	{ pinmux(5), 8, 0 },
	{ pinmux(8), 8, 7 },	/* GP3[0] */
	{ pinmux(8), 8, 6 },
	{ pinmux(8), 8, 5 },
	{ pinmux(8), 8, 4 },
	{ pinmux(8), 8, 3 },
	{ pinmux(8), 8, 2 },
	{ pinmux(8), 8, 1 },
	{ pinmux(8), 8, 0 },
	{ pinmux(7), 8, 7 },
	{ pinmux(7), 8, 6 },
	{ pinmux(7), 8, 5 },
	{ pinmux(7), 8, 4 },
	{ pinmux(7), 8, 3 },
	{ pinmux(7), 8, 2 },
	{ pinmux(7), 8, 1 },
	{ pinmux(7), 8, 0 },
	{ pinmux(10), 8, 7 },	/* GP4[0] */
	{ pinmux(10), 8, 6 },
	{ pinmux(10), 8, 5 },
	{ pinmux(10), 8, 4 },
	{ pinmux(10), 8, 3 },
	{ pinmux(10), 8, 2 },
	{ pinmux(10), 8, 1 },
	{ pinmux(10), 8, 0 },
	{ pinmux(9), 8, 7 },
	{ pinmux(9), 8, 6 },
	{ pinmux(9), 8, 5 },
	{ pinmux(9), 8, 4 },
	{ pinmux(9), 8, 3 },
	{ pinmux(9), 8, 2 },
	{ pinmux(9), 8, 1 },
	{ pinmux(9), 8, 0 },
	{ pinmux(12), 8, 7 },	/* GP5[0] */
	{ pinmux(12), 8, 6 },
	{ pinmux(12), 8, 5 },
	{ pinmux(12), 8, 4 },
	{ pinmux(12), 8, 3 },
	{ pinmux(12), 8, 2 },
	{ pinmux(12), 8, 1 },
	{ pinmux(12), 8, 0 },
	{ pinmux(11), 8, 7 },
	{ pinmux(11), 8, 6 },
	{ pinmux(11), 8, 5 },
	{ pinmux(11), 8, 4 },
	{ pinmux(11), 8, 3 },
	{ pinmux(11), 8, 2 },
	{ pinmux(11), 8, 1 },
	{ pinmux(11), 8, 0 },
	{ pinmux(19), 8, 6 },	/* GP6[0] */
	{ pinmux(19), 8, 5 },
	{ pinmux(19), 8, 4 },
	{ pinmux(19), 8, 3 },
	{ pinmux(19), 8, 2 },
	{ pinmux(16), 8, 1 },
	{ pinmux(14), 8, 1 },
	{ pinmux(14), 8, 0 },
	{ pinmux(13), 8, 7 },
	{ pinmux(13), 8, 6 },
	{ pinmux(13), 8, 5 },
	{ pinmux(13), 8, 4 },
	{ pinmux(13), 8, 3 },
	{ pinmux(13), 8, 2 },
	{ pinmux(13), 8, 1 },
	{ pinmux(13), 8, 0 },
	{ pinmux(18), 8, 1 },	/* GP7[0] */
	{ pinmux(18), 8, 0 },
	{ pinmux(17), 8, 7 },
	{ pinmux(17), 8, 6 },
	{ pinmux(17), 8, 5 },
	{ pinmux(17), 8, 4 },
	{ pinmux(17), 8, 3 },
	{ pinmux(17), 8, 2 },
	{ pinmux(17), 8, 1 },
	{ pinmux(17), 8, 0 },
	{ pinmux(16), 8, 7 },
	{ pinmux(16), 8, 6 },
	{ pinmux(16), 8, 5 },
	{ pinmux(16), 8, 4 },
	{ pinmux(16), 8, 3 },
	{ pinmux(16), 8, 2 },
	{ pinmux(19), 8, 0 },	/* GP8[0] */
	{ pinmux(3), 4, 7 },
	{ pinmux(3), 4, 6 },
	{ pinmux(3), 4, 5 },
	{ pinmux(3), 4, 4 },
	{ pinmux(3), 4, 3 },
	{ pinmux(3), 4, 2 },
	{ pinmux(2), 4, 7 },
	{ pinmux(19), 8, 1 },
	{ pinmux(19), 8, 0 },
	{ pinmux(18), 8, 7 },
	{ pinmux(18), 8, 6 },
	{ pinmux(18), 8, 5 },
	{ pinmux(18), 8, 4 },
	{ pinmux(18), 8, 3 },
	{ pinmux(18), 8, 2 },
};



int gpio_request(int gp, const char *label)
{
	if (gp >= MAX_NUM_GPIOS)
		return -1;

	if (gpio_registry[gp].is_registered)
		return -1;

	gpio_registry[gp].is_registered = 1;
	strncpy(gpio_registry[gp].name, label, GPIO_NAME_SIZE);
	gpio_registry[gp].name[GPIO_NAME_SIZE - 1] = 0;

	davinci_configure_pin_mux(&gpio_pinmux[gp], 1);

	return 0;
}


void gpio_free(int gp)
{
	gpio_registry[gp].is_registered = 0;
}


void gpio_toggle_value(int gp)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gp);
	gpio_set_value(gp, !gpio_get_value(gp));
}


int gpio_direction_input(int gp)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gp);
	setbits_le32(&bank->dir, 1U << GPIO_BIT(gp));
	return 0;
}


int gpio_direction_output(int gp, int value)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gp);
	clrbits_le32(&bank->dir, 1U << GPIO_BIT(gp));
	gpio_set_value(gp, value);
	return 0;
}


int gpio_get_value(int gp)
{
	struct davinci_gpio *bank;
	unsigned int ip;

	bank = GPIO_BANK(gp);
	ip = in_le32(&bank->in_data) & (1U << GPIO_BIT(gp));
	return ip ? 1 : 0;
}


void gpio_set_value(int gp, int value)
{
	struct davinci_gpio *bank;

	bank = GPIO_BANK(gp);

	if (value)
		bank->set_data = 1U << GPIO_BIT(gp);
	else
		bank->clr_data = 1U << GPIO_BIT(gp);
}


void gpio_info(void)
{
	int gp, dir, val;
	struct davinci_gpio *bank;

	for (gp = 0; gp < MAX_NUM_GPIOS; ++gp) {
		bank = GPIO_BANK(gp);
		dir = in_le32(&bank->dir) & (1U << GPIO_BIT(gp));
		val = gpio_get_value(gp);

		printf("% 4d: %s: %d [%c] %s\n",
			gp, dir ? " in" : "out", val,
			gpio_registry[gp].is_registered ? 'x' : ' ',
			gpio_registry[gp].name);
	}
}

/*
 * NVIDIA Tegra2 GPIO handling.
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
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
 * Based on (mostly copied from) kw_gpio.c based Linux 2.6 kernel driver.
 * Tom Warren (twarren@nvidia.com)
 */

#include <common.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/arch/tegra2.h>
#include <asm/gpio.h>

enum {
	TEGRA2_CMD_INFO,
	TEGRA2_CMD_PORT,
	TEGRA2_CMD_OUTPUT,
	TEGRA2_CMD_INPUT,
};

static struct gpio_names {
	char name[GPIO_NAME_SIZE];
} gpio_names[MAX_NUM_GPIOS];

static char *get_name(int i)
{
	return *gpio_names[i].name ? gpio_names[i].name : "UNKNOWN";
}

/* Return config of pin 'gp' as GPIO (1) or SFPIO (0) */
static int get_config(int gp)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	u32 u;
	int type;

	u = readl(&bank->gpio_config[GPIO_PORT(gp)]);
	type =  (u >> GPIO_BIT(gp)) & 1;

	debug("get_config: port = %d, bit = %d is %s\n",
		GPIO_FULLPORT(gp), GPIO_BIT(gp), type ? "GPIO" : "SFPIO");

	return type;
}

/* Config pin 'gp' as GPIO or SFPIO, based on 'type' */
static void set_config(int gp, int type)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	u32 u;

	debug("set_config: port = %d, bit = %d, %s\n",
		GPIO_FULLPORT(gp), GPIO_BIT(gp), type ? "GPIO" : "SFPIO");

	u = readl(&bank->gpio_config[GPIO_PORT(gp)]);
	if (type)				/* GPIO */
		u |= 1 << GPIO_BIT(gp);
	else
		u &= ~(1 << GPIO_BIT(gp));
	writel(u, &bank->gpio_config[GPIO_PORT(gp)]);
}

/* Return GPIO pin 'gp' direction - 0 = input or 1 = output */
static int get_direction(int gp)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	u32 u;
	int dir;

	u = readl(&bank->gpio_dir_out[GPIO_PORT(gp)]);
	dir =  (u >> GPIO_BIT(gp)) & 1;

	debug("get_direction: port = %d, bit = %d, %s\n",
		GPIO_FULLPORT(gp), GPIO_BIT(gp), dir ? "OUT" : "IN");

	return dir;
}

/* Config GPIO pin 'gp' as input or output (OE) as per 'output' */
static void set_direction(int gp, int output)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	u32 u;

	debug("set_direction: port = %d, bit = %d, %s\n",
		GPIO_FULLPORT(gp), GPIO_BIT(gp), output ? "OUT" : "IN");

	u = readl(&bank->gpio_dir_out[GPIO_PORT(gp)]);
	if (output)
		u |= 1 << GPIO_BIT(gp);
	else
		u &= ~(1 << GPIO_BIT(gp));
	writel(u, &bank->gpio_dir_out[GPIO_PORT(gp)]);
}

/* set GPIO pin 'gp' output bit as 0 or 1 as per 'high' */
static void set_level(int gp, int high)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	u32 u;

	debug("set_level: port = %d, bit %d == %d\n",
		GPIO_FULLPORT(gp), GPIO_BIT(gp), high);

	u = readl(&bank->gpio_out[GPIO_PORT(gp)]);
	if (high)
		u |= 1 << GPIO_BIT(gp);
	else
		u &= ~(1 << GPIO_BIT(gp));
	writel(u, &bank->gpio_out[GPIO_PORT(gp)]);
}

/*
 * Generic_GPIO primitives.
 */

int gpio_request(int gp, const char *label)
{
	if (gp >= MAX_NUM_GPIOS)
		return -1;

	strncpy(gpio_names[gp].name, label, GPIO_NAME_SIZE);
	gpio_names[gp].name[GPIO_NAME_SIZE - 1] = '\0';

	/* Configure as a GPIO */
	set_config(gp, 1);

	return 0;
}

void gpio_free(int gp)
{
}

/* read GPIO OUT value of pin 'gp' */
static int gpio_get_output_value(int gp)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	int val;

	debug("gpio_get_output_value: pin = %d (port %d:bit %d)\n",
		gp, GPIO_FULLPORT(gp), GPIO_BIT(gp));

	val = readl(&bank->gpio_out[GPIO_PORT(gp)]);

	return (val >> GPIO_BIT(gp)) & 1;
}

void gpio_toggle_value(int gp)
{
	gpio_set_value(gp, !gpio_get_output_value(gp));
}

/* set GPIO pin 'gp' as an input */
int gpio_direction_input(int gp)
{
	debug("gpio_direction_input: pin = %d (port %d:bit %d)\n",
		gp, GPIO_FULLPORT(gp), GPIO_BIT(gp));

	/* Configure GPIO direction as input. */
	set_direction(gp, 0);

	return 0;
}

/* set GPIO pin 'gp' as an output, with polarity 'value' */
int gpio_direction_output(int gp, int value)
{
	debug("gpio_direction_output: pin = %d (port %d:bit %d) = %s\n",
		gp, GPIO_FULLPORT(gp), GPIO_BIT(gp), value ? "HIGH" : "LOW");

	/* Configure GPIO output value. */
	set_level(gp, value);

	/* Configure GPIO direction as output. */
	set_direction(gp, 1);

	return 0;
}

/* read GPIO IN value of pin 'gp' */
int gpio_get_value(int gp)
{
	struct gpio_ctlr *gpio = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &gpio->gpio_bank[GPIO_BANK(gp)];
	int val;

	debug("gpio_get_value: pin = %d (port %d:bit %d)\n",
		gp, GPIO_FULLPORT(gp), GPIO_BIT(gp));

	val = readl(&bank->gpio_in[GPIO_PORT(gp)]);

	return (val >> GPIO_BIT(gp)) & 1;
}

/* write GPIO OUT value to pin 'gp' */
void gpio_set_value(int gp, int value)
{
	debug("gpio_set_value: pin = %d (port %d:bit %d), value = %d\n",
		gp, GPIO_FULLPORT(gp), GPIO_BIT(gp), value);

	/* Configure GPIO output value. */
	set_level(gp, value);
}

/*
 * Display Tegra GPIO information
 */
void gpio_info(void)
{
	int c, type;

	for (c = 0; c < MAX_NUM_GPIOS; c++) {
		type = get_config(c);		/* GPIO, not SFPIO */
		if (type) {
			printf("GPIO_%d:\t%s is an %s, ", c,
				get_name(c),
				get_direction(c) ? "OUTPUT" : "INPUT");
			if (get_direction(c))
				printf("value = %d", gpio_get_output_value(c));
			else
				printf("value = %d", gpio_get_value(c));
			printf("\n");
		} else
			continue;
	}
}

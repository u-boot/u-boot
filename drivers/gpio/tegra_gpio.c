/*
 * NVIDIA Tegra20 GPIO handling.
 *  (C) Copyright 2010-2012
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Based on (mostly copied from) kw_gpio.c based Linux 2.6 kernel driver.
 * Tom Warren (twarren@nvidia.com)
 */

#include <common.h>
#include <asm/io.h>
#include <asm/bitops.h>
#include <asm/arch/tegra.h>
#include <asm/gpio.h>

enum {
	TEGRA_CMD_INFO,
	TEGRA_CMD_PORT,
	TEGRA_CMD_OUTPUT,
	TEGRA_CMD_INPUT,
};

static struct gpio_names {
	char name[GPIO_NAME_SIZE];
} gpio_names[MAX_NUM_GPIOS];

static char *get_name(int i)
{
	return *gpio_names[i].name ? gpio_names[i].name : "UNKNOWN";
}

/* Return config of pin 'gpio' as GPIO (1) or SFPIO (0) */
static int get_config(unsigned gpio)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	u32 u;
	int type;

	u = readl(&bank->gpio_config[GPIO_PORT(gpio)]);
	type =  (u >> GPIO_BIT(gpio)) & 1;

	debug("get_config: port = %d, bit = %d is %s\n",
		GPIO_FULLPORT(gpio), GPIO_BIT(gpio), type ? "GPIO" : "SFPIO");

	return type;
}

/* Config pin 'gpio' as GPIO or SFPIO, based on 'type' */
static void set_config(unsigned gpio, int type)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	u32 u;

	debug("set_config: port = %d, bit = %d, %s\n",
		GPIO_FULLPORT(gpio), GPIO_BIT(gpio), type ? "GPIO" : "SFPIO");

	u = readl(&bank->gpio_config[GPIO_PORT(gpio)]);
	if (type)				/* GPIO */
		u |= 1 << GPIO_BIT(gpio);
	else
		u &= ~(1 << GPIO_BIT(gpio));
	writel(u, &bank->gpio_config[GPIO_PORT(gpio)]);
}

/* Return GPIO pin 'gpio' direction - 0 = input or 1 = output */
static int get_direction(unsigned gpio)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	u32 u;
	int dir;

	u = readl(&bank->gpio_dir_out[GPIO_PORT(gpio)]);
	dir =  (u >> GPIO_BIT(gpio)) & 1;

	debug("get_direction: port = %d, bit = %d, %s\n",
		GPIO_FULLPORT(gpio), GPIO_BIT(gpio), dir ? "OUT" : "IN");

	return dir;
}

/* Config GPIO pin 'gpio' as input or output (OE) as per 'output' */
static void set_direction(unsigned gpio, int output)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	u32 u;

	debug("set_direction: port = %d, bit = %d, %s\n",
		GPIO_FULLPORT(gpio), GPIO_BIT(gpio), output ? "OUT" : "IN");

	u = readl(&bank->gpio_dir_out[GPIO_PORT(gpio)]);
	if (output)
		u |= 1 << GPIO_BIT(gpio);
	else
		u &= ~(1 << GPIO_BIT(gpio));
	writel(u, &bank->gpio_dir_out[GPIO_PORT(gpio)]);
}

/* set GPIO pin 'gpio' output bit as 0 or 1 as per 'high' */
static void set_level(unsigned gpio, int high)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	u32 u;

	debug("set_level: port = %d, bit %d == %d\n",
		GPIO_FULLPORT(gpio), GPIO_BIT(gpio), high);

	u = readl(&bank->gpio_out[GPIO_PORT(gpio)]);
	if (high)
		u |= 1 << GPIO_BIT(gpio);
	else
		u &= ~(1 << GPIO_BIT(gpio));
	writel(u, &bank->gpio_out[GPIO_PORT(gpio)]);
}

/*
 * Generic_GPIO primitives.
 */

int gpio_request(unsigned gpio, const char *label)
{
	if (gpio >= MAX_NUM_GPIOS)
		return -1;

	if (label != NULL) {
		strncpy(gpio_names[gpio].name, label, GPIO_NAME_SIZE);
		gpio_names[gpio].name[GPIO_NAME_SIZE - 1] = '\0';
	}

	/* Configure as a GPIO */
	set_config(gpio, 1);

	return 0;
}

int gpio_free(unsigned gpio)
{
	if (gpio >= MAX_NUM_GPIOS)
		return -1;

	gpio_names[gpio].name[0] = '\0';
	/* Do not configure as input or change pin mux here */
	return 0;
}

/* read GPIO OUT value of pin 'gpio' */
static int gpio_get_output_value(unsigned gpio)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	int val;

	debug("gpio_get_output_value: pin = %d (port %d:bit %d)\n",
		gpio, GPIO_FULLPORT(gpio), GPIO_BIT(gpio));

	val = readl(&bank->gpio_out[GPIO_PORT(gpio)]);

	return (val >> GPIO_BIT(gpio)) & 1;
}

/* set GPIO pin 'gpio' as an input */
int gpio_direction_input(unsigned gpio)
{
	debug("gpio_direction_input: pin = %d (port %d:bit %d)\n",
		gpio, GPIO_FULLPORT(gpio), GPIO_BIT(gpio));

	/* Configure GPIO direction as input. */
	set_direction(gpio, 0);

	return 0;
}

/* set GPIO pin 'gpio' as an output, with polarity 'value' */
int gpio_direction_output(unsigned gpio, int value)
{
	debug("gpio_direction_output: pin = %d (port %d:bit %d) = %s\n",
		gpio, GPIO_FULLPORT(gpio), GPIO_BIT(gpio),
		value ? "HIGH" : "LOW");

	/* Configure GPIO output value. */
	set_level(gpio, value);

	/* Configure GPIO direction as output. */
	set_direction(gpio, 1);

	return 0;
}

/* read GPIO IN value of pin 'gpio' */
int gpio_get_value(unsigned gpio)
{
	struct gpio_ctlr *ctlr = (struct gpio_ctlr *)NV_PA_GPIO_BASE;
	struct gpio_ctlr_bank *bank = &ctlr->gpio_bank[GPIO_BANK(gpio)];
	int val;

	debug("gpio_get_value: pin = %d (port %d:bit %d)\n",
		gpio, GPIO_FULLPORT(gpio), GPIO_BIT(gpio));

	val = readl(&bank->gpio_in[GPIO_PORT(gpio)]);

	return (val >> GPIO_BIT(gpio)) & 1;
}

/* write GPIO OUT value to pin 'gpio' */
int gpio_set_value(unsigned gpio, int value)
{
	debug("gpio_set_value: pin = %d (port %d:bit %d), value = %d\n",
		gpio, GPIO_FULLPORT(gpio), GPIO_BIT(gpio), value);

	/* Configure GPIO output value. */
	set_level(gpio, value);

	return 0;
}

/*
 * Display Tegra GPIO information
 */
void gpio_info(void)
{
	unsigned c;
	int type;

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

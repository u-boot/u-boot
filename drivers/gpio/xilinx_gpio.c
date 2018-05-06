// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Xilinx, Michal Simek
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <linux/list.h>
#include <asm/io.h>
#include <asm/gpio.h>

static LIST_HEAD(gpio_list);

enum gpio_direction {
	GPIO_DIRECTION_OUT = 0,
	GPIO_DIRECTION_IN = 1,
};

/* Gpio simple map */
struct gpio_regs {
	u32 gpiodata;
	u32 gpiodir;
};

#define GPIO_NAME_SIZE	10

struct gpio_names {
	char name[GPIO_NAME_SIZE];
};

/* Initialized, rxbd_current, rx_first_buf must be 0 after init */
struct xilinx_gpio_priv {
	struct gpio_regs *regs;
	u32 gpio_min;
	u32 gpio_max;
	u32 gpiodata_store;
	char name[GPIO_NAME_SIZE];
	struct list_head list;
	struct gpio_names *gpio_name;
};

/* Store number of allocated gpio pins */
static u32 xilinx_gpio_max;

/* Get associated gpio controller */
static struct xilinx_gpio_priv *gpio_get_controller(unsigned gpio)
{
	struct list_head *entry;
	struct xilinx_gpio_priv *priv = NULL;

	list_for_each(entry, &gpio_list) {
		priv = list_entry(entry, struct xilinx_gpio_priv, list);
		if (gpio >= priv->gpio_min && gpio <= priv->gpio_max) {
			debug("%s: reg: %x, min-max: %d-%d\n", __func__,
			      (u32)priv->regs, priv->gpio_min, priv->gpio_max);
			return priv;
		}
	}
	puts("!!!Can't get gpio controller!!!\n");
	return NULL;
}

/* Get gpio pin name if used/setup */
static char *get_name(unsigned gpio)
{
	u32 gpio_priv;
	struct xilinx_gpio_priv *priv;

	debug("%s\n", __func__);

	priv = gpio_get_controller(gpio);
	if (priv) {
		gpio_priv = gpio - priv->gpio_min;

		return *priv->gpio_name[gpio_priv].name ?
			priv->gpio_name[gpio_priv].name : "UNKNOWN";
	}
	return "UNKNOWN";
}

/* Get output value */
static int gpio_get_output_value(unsigned gpio)
{
	u32 val, gpio_priv;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		gpio_priv = gpio - priv->gpio_min;
		val = !!(priv->gpiodata_store & (1 << gpio_priv));
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return val;
	}
	return -1;
}

/* Get input value */
static int gpio_get_input_value(unsigned gpio)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		gpio_priv = gpio - priv->gpio_min;
		val = readl(&regs->gpiodata);
		val = !!(val & (1 << gpio_priv));
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return val;
	}
	return -1;
}

/* Set gpio direction */
static int gpio_set_direction(unsigned gpio, enum gpio_direction direction)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		val = readl(&regs->gpiodir);

		gpio_priv = gpio - priv->gpio_min;
		if (direction == GPIO_DIRECTION_OUT)
			val &= ~(1 << gpio_priv);
		else
			val |= 1 << gpio_priv;

		writel(val, &regs->gpiodir);
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return 0;
	}

	return -1;
}

/* Get gpio direction */
static int gpio_get_direction(unsigned gpio)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		gpio_priv = gpio - priv->gpio_min;
		val = readl(&regs->gpiodir);
		val = !!(val & (1 << gpio_priv));
		debug("%s: reg: %x, gpio_no: %d, dir: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);

		return val;
	}

	return -1;
}

/*
 * Get input value
 * for example gpio setup to output only can't get input value
 * which is breaking gpio toggle command
 */
int gpio_get_value(unsigned gpio)
{
	u32 val;

	if (gpio_get_direction(gpio) == GPIO_DIRECTION_OUT)
		val = gpio_get_output_value(gpio);
	else
		val = gpio_get_input_value(gpio);

	return val;
}

/* Set output value */
static int gpio_set_output_value(unsigned gpio, int value)
{
	u32 val, gpio_priv;
	struct gpio_regs *regs;
	struct xilinx_gpio_priv *priv = gpio_get_controller(gpio);

	if (priv) {
		regs = priv->regs;
		gpio_priv = gpio - priv->gpio_min;
		val = priv->gpiodata_store;
		if (value)
			val |= 1 << gpio_priv;
		else
			val &= ~(1 << gpio_priv);

		writel(val, &regs->gpiodata);
		debug("%s: reg: %x, gpio_no: %d, output_val: %d\n", __func__,
		      (u32)priv->regs, gpio_priv, val);
		priv->gpiodata_store = val;

		return 0;
	}

	return -1;
}

int gpio_set_value(unsigned gpio, int value)
{
	if (gpio_get_direction(gpio) == GPIO_DIRECTION_OUT)
		return gpio_set_output_value(gpio, value);

	return -1;
}

/* Set GPIO as input */
int gpio_direction_input(unsigned gpio)
{
	debug("%s\n", __func__);
	return gpio_set_direction(gpio, GPIO_DIRECTION_IN);
}

/* Setup GPIO as output and set output value */
int gpio_direction_output(unsigned gpio, int value)
{
	int ret = gpio_set_direction(gpio, GPIO_DIRECTION_OUT);

	debug("%s\n", __func__);

	if (ret < 0)
		return ret;

	return gpio_set_output_value(gpio, value);
}

/* Show gpio status */
void gpio_info(void)
{
	unsigned gpio;

	struct list_head *entry;
	struct xilinx_gpio_priv *priv = NULL;

	list_for_each(entry, &gpio_list) {
		priv = list_entry(entry, struct xilinx_gpio_priv, list);
		printf("\n%s: %s/%x (%d-%d)\n", __func__, priv->name,
		       (u32)priv->regs, priv->gpio_min, priv->gpio_max);

		for (gpio = priv->gpio_min; gpio <= priv->gpio_max; gpio++) {
			printf("GPIO_%d:\t%s is an ", gpio, get_name(gpio));
			if (gpio_get_direction(gpio) == GPIO_DIRECTION_OUT)
				printf("OUTPUT value = %d\n",
				       gpio_get_output_value(gpio));
			else
				printf("INPUT value = %d\n",
				       gpio_get_input_value(gpio));
		}
	}
}

int gpio_request(unsigned gpio, const char *label)
{
	u32 gpio_priv;
	struct xilinx_gpio_priv *priv;

	if (gpio >= xilinx_gpio_max)
		return -EINVAL;

	priv = gpio_get_controller(gpio);
	if (priv) {
		gpio_priv = gpio - priv->gpio_min;

		if (label != NULL) {
			strncpy(priv->gpio_name[gpio_priv].name, label,
				GPIO_NAME_SIZE);
			priv->gpio_name[gpio_priv].name[GPIO_NAME_SIZE - 1] =
					'\0';
		}
		return 0;
	}

	return -1;
}

int gpio_free(unsigned gpio)
{
	u32 gpio_priv;
	struct xilinx_gpio_priv *priv;

	if (gpio >= xilinx_gpio_max)
		return -EINVAL;

	priv = gpio_get_controller(gpio);
	if (priv) {
		gpio_priv = gpio - priv->gpio_min;
		priv->gpio_name[gpio_priv].name[0] = '\0';

		/* Do nothing here */
		return 0;
	}

	return -1;
}

int gpio_alloc(u32 baseaddr, const char *name, u32 gpio_no)
{
	struct xilinx_gpio_priv *priv;

	priv = calloc(1, sizeof(struct xilinx_gpio_priv));

	/* Setup gpio name */
	if (name != NULL) {
		strncpy(priv->name, name, GPIO_NAME_SIZE);
		priv->name[GPIO_NAME_SIZE - 1] = '\0';
	}
	priv->regs = (struct gpio_regs *)baseaddr;

	priv->gpio_min = xilinx_gpio_max;
	xilinx_gpio_max = priv->gpio_min + gpio_no;
	priv->gpio_max = xilinx_gpio_max - 1;

	priv->gpio_name = calloc(gpio_no, sizeof(struct gpio_names));

	INIT_LIST_HEAD(&priv->list);
	list_add_tail(&priv->list, &gpio_list);

	printf("%s: Add %s (%d-%d)\n", __func__, name,
	       priv->gpio_min, priv->gpio_max);

	/* Return the first gpio allocated for this device */
	return priv->gpio_min;
}

/* Dual channel gpio is one IP with two independent channels */
int gpio_alloc_dual(u32 baseaddr, const char *name, u32 gpio_no0, u32 gpio_no1)
{
	int ret;

	ret = gpio_alloc(baseaddr, name, gpio_no0);
	gpio_alloc(baseaddr + 8, strcat((char *)name, "_1"), gpio_no1);

	/* Return the first gpio allocated for this device */
	return ret;
}

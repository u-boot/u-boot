/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

/* Flags for each GPIO */
#define GPIOF_OUTPUT	(1 << 0)	/* Currently set as an output */
#define GPIOF_HIGH	(1 << 1)	/* Currently set high */
#define GPIOF_RESERVED	(1 << 2)	/* Is in use / requested */

struct gpio_state {
	const char *label;	/* label given by requester */
	u8 flags;		/* flags (GPIOF_...) */
};

/* Access routines for GPIO state */
static u8 *get_gpio_flags(struct device *dev, unsigned offset)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct gpio_state *state = dev_get_priv(dev);

	if (offset >= uc_priv->gpio_count) {
		static u8 invalid_flags;
		printf("sandbox_gpio: error: invalid gpio %u\n", offset);
		return &invalid_flags;
	}

	return &state[offset].flags;
}

static int get_gpio_flag(struct device *dev, unsigned offset, int flag)
{
	return (*get_gpio_flags(dev, offset) & flag) != 0;
}

static int set_gpio_flag(struct device *dev, unsigned offset, int flag,
			 int value)
{
	u8 *gpio = get_gpio_flags(dev, offset);

	if (value)
		*gpio |= flag;
	else
		*gpio &= ~flag;

	return 0;
}

static int check_reserved(struct device *dev, unsigned offset,
			  const char *func)
{
	if (!get_gpio_flag(dev, offset, GPIOF_RESERVED)) {
		printf("sandbox_gpio: %s: error: offset %u not reserved\n",
		       func, offset);
		return -1;
	}

	return 0;
}

/*
 * Back-channel sandbox-internal-only access to GPIO state
 */

int sandbox_gpio_get_value(struct device *dev, unsigned offset)
{
	if (get_gpio_flag(dev, offset, GPIOF_OUTPUT))
		debug("sandbox_gpio: get_value on output gpio %u\n", offset);
	return get_gpio_flag(dev, offset, GPIOF_HIGH);
}

int sandbox_gpio_set_value(struct device *dev, unsigned offset, int value)
{
	return set_gpio_flag(dev, offset, GPIOF_HIGH, value);
}

int sandbox_gpio_get_direction(struct device *dev, unsigned offset)
{
	return get_gpio_flag(dev, offset, GPIOF_OUTPUT);
}

int sandbox_gpio_set_direction(struct device *dev, unsigned offset, int output)
{
	return set_gpio_flag(dev, offset, GPIOF_OUTPUT, output);
}

/*
 * These functions implement the public interface within U-Boot
 */

/* set GPIO port 'offset' as an input */
static int sb_gpio_direction_input(struct device *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	if (check_reserved(dev, offset, __func__))
		return -1;

	return sandbox_gpio_set_direction(dev, offset, 0);
}

/* set GPIO port 'offset' as an output, with polarity 'value' */
static int sb_gpio_direction_output(struct device *dev, unsigned offset,
				    int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	if (check_reserved(dev, offset, __func__))
		return -1;

	return sandbox_gpio_set_direction(dev, offset, 1) |
		sandbox_gpio_set_value(dev, offset, value);
}

/* read GPIO IN value of port 'offset' */
static int sb_gpio_get_value(struct device *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	if (check_reserved(dev, offset, __func__))
		return -1;

	return sandbox_gpio_get_value(dev, offset);
}

/* write GPIO OUT value to port 'offset' */
static int sb_gpio_set_value(struct device *dev, unsigned offset, int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	if (check_reserved(dev, offset, __func__))
		return -1;

	if (!sandbox_gpio_get_direction(dev, offset)) {
		printf("sandbox_gpio: error: set_value on input gpio %u\n",
		       offset);
		return -1;
	}

	return sandbox_gpio_set_value(dev, offset, value);
}

static int sb_gpio_request(struct device *dev, unsigned offset,
			   const char *label)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct gpio_state *state = dev_get_priv(dev);

	debug("%s: offset:%u, label:%s\n", __func__, offset, label);

	if (offset >= uc_priv->gpio_count) {
		printf("sandbox_gpio: error: invalid gpio %u\n", offset);
		return -1;
	}

	if (get_gpio_flag(dev, offset, GPIOF_RESERVED)) {
		printf("sandbox_gpio: error: gpio %u already reserved\n",
		       offset);
		return -1;
	}

	state[offset].label = label;
	return set_gpio_flag(dev, offset, GPIOF_RESERVED, 1);
}

static int sb_gpio_free(struct device *dev, unsigned offset)
{
	struct gpio_state *state = dev_get_priv(dev);

	debug("%s: offset:%u\n", __func__, offset);

	if (check_reserved(dev, offset, __func__))
		return -1;

	state[offset].label = NULL;
	return set_gpio_flag(dev, offset, GPIOF_RESERVED, 0);
}

static int sb_gpio_get_state(struct device *dev, unsigned int offset,
			     char *buf, int bufsize)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct gpio_state *state = dev_get_priv(dev);
	const char *label;

	label = state[offset].label;
	snprintf(buf, bufsize, "%s%d: %s: %d [%c]%s%s",
		 uc_priv->bank_name ? uc_priv->bank_name : "", offset,
		 sandbox_gpio_get_direction(dev, offset) ? "out" : " in",
		 sandbox_gpio_get_value(dev, offset),
		 get_gpio_flag(dev, offset, GPIOF_RESERVED) ? 'x' : ' ',
		 label ? " " : "",
		 label ? label : "");

	return 0;
}

static const struct dm_gpio_ops gpio_sandbox_ops = {
	.request		= sb_gpio_request,
	.free			= sb_gpio_free,
	.direction_input	= sb_gpio_direction_input,
	.direction_output	= sb_gpio_direction_output,
	.get_value		= sb_gpio_get_value,
	.set_value		= sb_gpio_set_value,
	.get_state		= sb_gpio_get_state,
};

static int sandbox_gpio_ofdata_to_platdata(struct device *dev)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

	uc_priv->gpio_count = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					     "num-gpios", 0);
	uc_priv->bank_name = fdt_getprop(gd->fdt_blob, dev->of_offset,
					 "gpio-bank-name", NULL);

	return 0;
}

static int gpio_sandbox_probe(struct device *dev)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

	if (dev->of_offset == -1) {
		/* Tell the uclass how many GPIOs we have */
		uc_priv->gpio_count = CONFIG_SANDBOX_GPIO_COUNT;
	}

	dev->priv = calloc(sizeof(struct gpio_state), uc_priv->gpio_count);

	return 0;
}

static const struct device_id sandbox_gpio_ids[] = {
	{ .compatible = "sandbox,gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_sandbox) = {
	.name	= "gpio_sandbox",
	.id	= UCLASS_GPIO,
	.of_match = sandbox_gpio_ids,
	.ofdata_to_platdata = sandbox_gpio_ofdata_to_platdata,
	.probe	= gpio_sandbox_probe,
	.ops	= &gpio_sandbox_ops,
};

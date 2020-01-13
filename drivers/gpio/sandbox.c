// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <dm/lists.h>
#include <dm/of.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/gpio.h>
#include <dt-bindings/gpio/sandbox-gpio.h>


struct gpio_state {
	const char *label;	/* label given by requester */
	ulong dir_flags;	/* dir_flags (GPIOD_...) */
};

/* Access routines for GPIO dir flags */
static ulong *get_gpio_dir_flags(struct udevice *dev, unsigned int offset)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct gpio_state *state = dev_get_priv(dev);

	if (offset >= uc_priv->gpio_count) {
		static ulong invalid_dir_flags;
		printf("sandbox_gpio: error: invalid gpio %u\n", offset);
		return &invalid_dir_flags;
	}

	return &state[offset].dir_flags;

}

static int get_gpio_flag(struct udevice *dev, unsigned int offset, ulong flag)
{
	return (*get_gpio_dir_flags(dev, offset) & flag) != 0;
}

static int set_gpio_flag(struct udevice *dev, unsigned int offset, ulong flag,
			 int value)
{
	ulong *gpio = get_gpio_dir_flags(dev, offset);

	if (value)
		*gpio |= flag;
	else
		*gpio &= ~flag;

	return 0;
}

/*
 * Back-channel sandbox-internal-only access to GPIO state
 */

int sandbox_gpio_get_value(struct udevice *dev, unsigned offset)
{
	if (get_gpio_flag(dev, offset, GPIOD_IS_OUT))
		debug("sandbox_gpio: get_value on output gpio %u\n", offset);
	return get_gpio_flag(dev, offset, GPIOD_IS_OUT_ACTIVE);
}

int sandbox_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	return set_gpio_flag(dev, offset, GPIOD_IS_OUT_ACTIVE, value);
}

int sandbox_gpio_get_direction(struct udevice *dev, unsigned offset)
{
	return get_gpio_flag(dev, offset, GPIOD_IS_OUT);
}

int sandbox_gpio_set_direction(struct udevice *dev, unsigned offset, int output)
{
	set_gpio_flag(dev, offset, GPIOD_IS_OUT, output);
	set_gpio_flag(dev, offset, GPIOD_IS_IN, !(output));

	return 0;
}

ulong sandbox_gpio_get_dir_flags(struct udevice *dev, unsigned int offset)
{
	return *get_gpio_dir_flags(dev, offset);
}

int sandbox_gpio_set_dir_flags(struct udevice *dev, unsigned int offset,
			       ulong flags)
{
	*get_gpio_dir_flags(dev, offset) = flags;

	return 0;
}

/*
 * These functions implement the public interface within U-Boot
 */

/* set GPIO port 'offset' as an input */
static int sb_gpio_direction_input(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_set_direction(dev, offset, 0);
}

/* set GPIO port 'offset' as an output, with polarity 'value' */
static int sb_gpio_direction_output(struct udevice *dev, unsigned offset,
				    int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	return sandbox_gpio_set_direction(dev, offset, 1) |
		sandbox_gpio_set_value(dev, offset, value);
}

/* read GPIO IN value of port 'offset' */
static int sb_gpio_get_value(struct udevice *dev, unsigned offset)
{
	debug("%s: offset:%u\n", __func__, offset);

	return sandbox_gpio_get_value(dev, offset);
}

/* write GPIO OUT value to port 'offset' */
static int sb_gpio_set_value(struct udevice *dev, unsigned offset, int value)
{
	debug("%s: offset:%u, value = %d\n", __func__, offset, value);

	if (!sandbox_gpio_get_direction(dev, offset)) {
		printf("sandbox_gpio: error: set_value on input gpio %u\n",
		       offset);
		return -1;
	}

	return sandbox_gpio_set_value(dev, offset, value);
}

static int sb_gpio_get_function(struct udevice *dev, unsigned offset)
{
	if (get_gpio_flag(dev, offset, GPIOD_IS_OUT))
		return GPIOF_OUTPUT;
	if (get_gpio_flag(dev, offset, GPIOD_IS_IN))
		return GPIOF_INPUT;

	return GPIOF_INPUT; /*GPIO is not configurated */
}

static int sb_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			 struct ofnode_phandle_args *args)
{
	desc->offset = args->args[0];
	if (args->args_count < 2)
		return 0;
	/* treat generic binding with gpio uclass */
	gpio_xlate_offs_flags(dev, desc, args);

	/* sandbox test specific, not defined in gpio.h */
	if (args->args[1] & GPIO_IN)
		desc->flags |= GPIOD_IS_IN;

	if (args->args[1] & GPIO_OUT)
		desc->flags |= GPIOD_IS_OUT;

	if (args->args[1] & GPIO_OUT_ACTIVE)
		desc->flags |= GPIOD_IS_OUT_ACTIVE;

	return 0;
}

static int sb_gpio_set_dir_flags(struct udevice *dev, unsigned int offset,
				 ulong flags)
{
	ulong *dir_flags;

	debug("%s: offset:%u, dir_flags = %lx\n", __func__, offset, flags);

	dir_flags = get_gpio_dir_flags(dev, offset);

	*dir_flags = flags;

	return 0;
}

static int sb_gpio_get_dir_flags(struct udevice *dev, unsigned int offset,
				 ulong *flags)
{
	debug("%s: offset:%u\n", __func__, offset);
	*flags = *get_gpio_dir_flags(dev, offset);

	return 0;
}

static const struct dm_gpio_ops gpio_sandbox_ops = {
	.direction_input	= sb_gpio_direction_input,
	.direction_output	= sb_gpio_direction_output,
	.get_value		= sb_gpio_get_value,
	.set_value		= sb_gpio_set_value,
	.get_function		= sb_gpio_get_function,
	.xlate			= sb_gpio_xlate,
	.set_dir_flags		= sb_gpio_set_dir_flags,
	.get_dir_flags		= sb_gpio_get_dir_flags,
};

static int sandbox_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = dev_read_u32_default(dev, "sandbox,gpio-count",
						   0);
	uc_priv->bank_name = dev_read_string(dev, "gpio-bank-name");

	return 0;
}

static int gpio_sandbox_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!dev_of_valid(dev))
		/* Tell the uclass how many GPIOs we have */
		uc_priv->gpio_count = CONFIG_SANDBOX_GPIO_COUNT;

	dev->priv = calloc(sizeof(struct gpio_state), uc_priv->gpio_count);

	return 0;
}

static int gpio_sandbox_remove(struct udevice *dev)
{
	free(dev->priv);

	return 0;
}

static const struct udevice_id sandbox_gpio_ids[] = {
	{ .compatible = "sandbox,gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_sandbox) = {
	.name	= "gpio_sandbox",
	.id	= UCLASS_GPIO,
	.of_match = sandbox_gpio_ids,
	.ofdata_to_platdata = sandbox_gpio_ofdata_to_platdata,
	.probe	= gpio_sandbox_probe,
	.remove	= gpio_sandbox_remove,
	.ops	= &gpio_sandbox_ops,
};

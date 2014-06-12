/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>

/**
 * gpio_to_device() - Convert global GPIO number to device, number
 * gpio:	The numeric representation of the GPIO
 *
 * Convert the GPIO number to an entry in the list of GPIOs
 * or GPIO blocks registered with the GPIO controller. Returns
 * entry on success, NULL on error.
 */
static int gpio_to_device(unsigned int gpio, struct udevice **devp,
			  unsigned int *offset)
{
	struct gpio_dev_priv *uc_priv;
	struct udevice *dev;
	int ret;

	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		uc_priv = dev->uclass_priv;
		if (gpio >= uc_priv->gpio_base &&
		    gpio < uc_priv->gpio_base + uc_priv->gpio_count) {
			*devp = dev;
			*offset = gpio - uc_priv->gpio_base;
			return 0;
		}
	}

	/* No such GPIO */
	return ret ? ret : -EINVAL;
}

int gpio_lookup_name(const char *name, struct udevice **devp,
		     unsigned int *offsetp, unsigned int *gpiop)
{
	struct gpio_dev_priv *uc_priv;
	struct udevice *dev;
	int ret;

	if (devp)
		*devp = NULL;
	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		ulong offset;
		int len;

		uc_priv = dev->uclass_priv;
		len = uc_priv->bank_name ? strlen(uc_priv->bank_name) : 0;

		if (!strncasecmp(name, uc_priv->bank_name, len)) {
			if (strict_strtoul(name + len, 10, &offset))
				continue;
			if (devp)
				*devp = dev;
			if (offsetp)
				*offsetp = offset;
			if (gpiop)
				*gpiop = uc_priv->gpio_base + offset;
			return 0;
		}
	}

	return ret ? ret : -EINVAL;
}

/**
 * gpio_request() - [COMPAT] Request GPIO
 * gpio:	GPIO number
 * label:	Name for the requested GPIO
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_request(unsigned gpio, const char *label)
{
	unsigned int offset;
	struct udevice *dev;
	int ret;

	ret = gpio_to_device(gpio, &dev, &offset);
	if (ret)
		return ret;

	if (!gpio_get_ops(dev)->request)
		return 0;

	return gpio_get_ops(dev)->request(dev, offset, label);
}

/**
 * gpio_free() - [COMPAT] Relinquish GPIO
 * gpio:	GPIO number
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_free(unsigned gpio)
{
	unsigned int offset;
	struct udevice *dev;
	int ret;

	ret = gpio_to_device(gpio, &dev, &offset);
	if (ret)
		return ret;

	if (!gpio_get_ops(dev)->free)
		return 0;
	return gpio_get_ops(dev)->free(dev, offset);
}

/**
 * gpio_direction_input() - [COMPAT] Set GPIO direction to input
 * gpio:	GPIO number
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_direction_input(unsigned gpio)
{
	unsigned int offset;
	struct udevice *dev;
	int ret;

	ret = gpio_to_device(gpio, &dev, &offset);
	if (ret)
		return ret;

	return gpio_get_ops(dev)->direction_input(dev, offset);
}

/**
 * gpio_direction_output() - [COMPAT] Set GPIO direction to output and set value
 * gpio:	GPIO number
 * value:	Logical value to be set on the GPIO pin
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_direction_output(unsigned gpio, int value)
{
	unsigned int offset;
	struct udevice *dev;
	int ret;

	ret = gpio_to_device(gpio, &dev, &offset);
	if (ret)
		return ret;

	return gpio_get_ops(dev)->direction_output(dev, offset, value);
}

/**
 * gpio_get_value() - [COMPAT] Sample GPIO pin and return it's value
 * gpio:	GPIO number
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns the value of the GPIO pin, or negative value
 * on error.
 */
int gpio_get_value(unsigned gpio)
{
	unsigned int offset;
	struct udevice *dev;
	int ret;

	ret = gpio_to_device(gpio, &dev, &offset);
	if (ret)
		return ret;

	return gpio_get_ops(dev)->get_value(dev, offset);
}

/**
 * gpio_set_value() - [COMPAT] Configure logical value on GPIO pin
 * gpio:	GPIO number
 * value:	Logical value to be set on the GPIO pin.
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_set_value(unsigned gpio, int value)
{
	unsigned int offset;
	struct udevice *dev;
	int ret;

	ret = gpio_to_device(gpio, &dev, &offset);
	if (ret)
		return ret;

	return gpio_get_ops(dev)->set_value(dev, offset, value);
}

const char *gpio_get_bank_info(struct udevice *dev, int *bit_count)
{
	struct gpio_dev_priv *priv;

	/* Must be called on an active device */
	priv = dev->uclass_priv;
	assert(priv);

	*bit_count = priv->gpio_count;
	return priv->bank_name;
}

/* We need to renumber the GPIOs when any driver is probed/removed */
static int gpio_renumber(void)
{
	struct gpio_dev_priv *uc_priv;
	struct udevice *dev;
	struct uclass *uc;
	unsigned base;
	int ret;

	ret = uclass_get(UCLASS_GPIO, &uc);
	if (ret)
		return ret;

	/* Ensure that we have a base for each bank */
	base = 0;
	uclass_foreach_dev(dev, uc) {
		if (device_active(dev)) {
			uc_priv = dev->uclass_priv;
			uc_priv->gpio_base = base;
			base += uc_priv->gpio_count;
		}
	}

	return 0;
}

static int gpio_post_probe(struct udevice *dev)
{
	return gpio_renumber();
}

static int gpio_pre_remove(struct udevice *dev)
{
	return gpio_renumber();
}

UCLASS_DRIVER(gpio) = {
	.id		= UCLASS_GPIO,
	.name		= "gpio",
	.post_probe	= gpio_post_probe,
	.pre_remove	= gpio_pre_remove,
	.per_device_auto_alloc_size = sizeof(struct gpio_dev_priv),
};

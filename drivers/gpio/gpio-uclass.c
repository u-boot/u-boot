/*
 * Copyright (c) 2013 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <linux/ctype.h>

/**
 * gpio_to_device() - Convert global GPIO number to device, number
 *
 * Convert the GPIO number to an entry in the list of GPIOs
 * or GPIO blocks registered with the GPIO controller. Returns
 * entry on success, NULL on error.
 *
 * @gpio:	The numeric representation of the GPIO
 * @desc:	Returns description (desc->flags will always be 0)
 * @return 0 if found, -ENOENT if not found
 */
static int gpio_to_device(unsigned int gpio, struct gpio_desc *desc)
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
			desc->dev = dev;
			desc->offset = gpio - uc_priv->gpio_base;
			desc->flags = 0;
			return 0;
		}
	}

	/* No such GPIO */
	return ret ? ret : -ENOENT;
}

int gpio_lookup_name(const char *name, struct udevice **devp,
		     unsigned int *offsetp, unsigned int *gpiop)
{
	struct gpio_dev_priv *uc_priv = NULL;
	struct udevice *dev;
	ulong offset;
	int numeric;
	int ret;

	if (devp)
		*devp = NULL;
	numeric = isdigit(*name) ? simple_strtoul(name, NULL, 10) : -1;
	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		int len;

		uc_priv = dev->uclass_priv;
		if (numeric != -1) {
			offset = numeric - uc_priv->gpio_base;
			/* Allow GPIOs to be numbered from 0 */
			if (offset >= 0 && offset < uc_priv->gpio_count)
				break;
		}

		len = uc_priv->bank_name ? strlen(uc_priv->bank_name) : 0;

		if (!strncasecmp(name, uc_priv->bank_name, len)) {
			if (!strict_strtoul(name + len, 10, &offset))
				break;
		}
	}

	if (!dev)
		return ret ? ret : -EINVAL;

	if (devp)
		*devp = dev;
	if (offsetp)
		*offsetp = offset;
	if (gpiop)
		*gpiop = uc_priv->gpio_base + offset;

	return 0;
}

static int dm_gpio_request(struct gpio_desc *desc, const char *label)
{
	struct udevice *dev = desc->dev;
	struct gpio_dev_priv *uc_priv;
	char *str;
	int ret;

	uc_priv = dev->uclass_priv;
	if (uc_priv->name[desc->offset])
		return -EBUSY;
	str = strdup(label);
	if (!str)
		return -ENOMEM;
	if (gpio_get_ops(dev)->request) {
		ret = gpio_get_ops(dev)->request(dev, desc->offset, label);
		if (ret) {
			free(str);
			return ret;
		}
	}
	uc_priv->name[desc->offset] = str;

	return 0;
}

/**
 * gpio_request() - [COMPAT] Request GPIO
 * gpio:	GPIO number
 * label:	Name for the requested GPIO
 *
 * The label is copied and allocated so the caller does not need to keep
 * the pointer around.
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_request(unsigned gpio, const char *label)
{
	struct gpio_desc desc;
	int ret;

	ret = gpio_to_device(gpio, &desc);
	if (ret)
		return ret;

	return dm_gpio_request(&desc, label);
}

/**
 * gpio_requestf() - [COMPAT] Request GPIO
 * @gpio:	GPIO number
 * @fmt:	Format string for the requested GPIO
 * @...:	Arguments for the printf() format string
 *
 * This function implements the API that's compatible with current
 * GPIO API used in U-Boot. The request is forwarded to particular
 * GPIO driver. Returns 0 on success, negative value on error.
 */
int gpio_requestf(unsigned gpio, const char *fmt, ...)
{
	va_list args;
	char buf[40];

	va_start(args, fmt);
	vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	return gpio_request(gpio, buf);
}

int _dm_gpio_free(struct udevice *dev, uint offset)
{
	struct gpio_dev_priv *uc_priv;
	int ret;

	uc_priv = dev->uclass_priv;
	if (!uc_priv->name[offset])
		return -ENXIO;
	if (gpio_get_ops(dev)->free) {
		ret = gpio_get_ops(dev)->free(dev, offset);
		if (ret)
			return ret;
	}

	free(uc_priv->name[offset]);
	uc_priv->name[offset] = NULL;

	return 0;
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
	struct gpio_desc desc;
	int ret;

	ret = gpio_to_device(gpio, &desc);
	if (ret)
		return ret;

	return _dm_gpio_free(desc.dev, desc.offset);
}

static int check_reserved(struct gpio_desc *desc, const char *func)
{
	struct gpio_dev_priv *uc_priv = desc->dev->uclass_priv;

	if (!uc_priv->name[desc->offset]) {
		printf("%s: %s: error: gpio %s%d not reserved\n",
		       desc->dev->name, func,
		       uc_priv->bank_name ? uc_priv->bank_name : "",
		       desc->offset);
		return -EBUSY;
	}

	return 0;
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
	struct gpio_desc desc;
	int ret;

	ret = gpio_to_device(gpio, &desc);
	if (ret)
		return ret;
	ret = check_reserved(&desc, "dir_input");
	if (ret)
		return ret;

	return gpio_get_ops(desc.dev)->direction_input(desc.dev, desc.offset);
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
	struct gpio_desc desc;
	int ret;

	ret = gpio_to_device(gpio, &desc);
	if (ret)
		return ret;
	ret = check_reserved(&desc, "dir_output");
	if (ret)
		return ret;

	return gpio_get_ops(desc.dev)->direction_output(desc.dev,
							desc.offset, value);
}

int dm_gpio_get_value(struct gpio_desc *desc)
{
	int value;
	int ret;

	ret = check_reserved(desc, "get_value");
	if (ret)
		return ret;

	value = gpio_get_ops(desc->dev)->get_value(desc->dev, desc->offset);

	return desc->flags & GPIOD_ACTIVE_LOW ? !value : value;
}

int dm_gpio_set_value(struct gpio_desc *desc, int value)
{
	int ret;

	ret = check_reserved(desc, "set_value");
	if (ret)
		return ret;

	if (desc->flags & GPIOD_ACTIVE_LOW)
		value = !value;
	gpio_get_ops(desc->dev)->set_value(desc->dev, desc->offset, value);
	return 0;
}

int dm_gpio_set_dir_flags(struct gpio_desc *desc, ulong flags)
{
	struct udevice *dev = desc->dev;
	struct dm_gpio_ops *ops = gpio_get_ops(dev);
	int ret;

	ret = check_reserved(desc, "set_dir");
	if (ret)
		return ret;

	if (flags & GPIOD_IS_OUT) {
		int value = flags & GPIOD_IS_OUT_ACTIVE ? 1 : 0;

		if (flags & GPIOD_ACTIVE_LOW)
			value = !value;
		ret = ops->direction_output(dev, desc->offset, value);
	} else  if (flags & GPIOD_IS_IN) {
		ret = ops->direction_input(dev, desc->offset);
	}
	if (ret)
		return ret;
	/*
	 * Update desc->flags here, so that GPIO_ACTIVE_LOW is honoured in
	 * futures
	 */
	desc->flags = flags;

	return 0;
}

int dm_gpio_set_dir(struct gpio_desc *desc)
{
	return dm_gpio_set_dir_flags(desc, desc->flags);
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
	int ret;

	struct gpio_desc desc;

	ret = gpio_to_device(gpio, &desc);
	if (ret)
		return ret;
	return dm_gpio_get_value(&desc);
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
	struct gpio_desc desc;
	int ret;

	ret = gpio_to_device(gpio, &desc);
	if (ret)
		return ret;
	return dm_gpio_set_value(&desc, value);
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

static const char * const gpio_function[GPIOF_COUNT] = {
	"input",
	"output",
	"unused",
	"unknown",
	"func",
};

int get_function(struct udevice *dev, int offset, bool skip_unused,
		 const char **namep)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	struct dm_gpio_ops *ops = gpio_get_ops(dev);

	BUILD_BUG_ON(GPIOF_COUNT != ARRAY_SIZE(gpio_function));
	if (!device_active(dev))
		return -ENODEV;
	if (offset < 0 || offset >= uc_priv->gpio_count)
		return -EINVAL;
	if (namep)
		*namep = uc_priv->name[offset];
	if (skip_unused && !uc_priv->name[offset])
		return GPIOF_UNUSED;
	if (ops->get_function) {
		int ret;

		ret = ops->get_function(dev, offset);
		if (ret < 0)
			return ret;
		if (ret >= ARRAY_SIZE(gpio_function))
			return -ENODATA;
		return ret;
	}

	return GPIOF_UNKNOWN;
}

int gpio_get_function(struct udevice *dev, int offset, const char **namep)
{
	return get_function(dev, offset, true, namep);
}

int gpio_get_raw_function(struct udevice *dev, int offset, const char **namep)
{
	return get_function(dev, offset, false, namep);
}

int gpio_get_status(struct udevice *dev, int offset, char *buf, int buffsize)
{
	struct dm_gpio_ops *ops = gpio_get_ops(dev);
	struct gpio_dev_priv *priv;
	char *str = buf;
	int func;
	int ret;
	int len;

	BUILD_BUG_ON(GPIOF_COUNT != ARRAY_SIZE(gpio_function));

	*buf = 0;
	priv = dev->uclass_priv;
	ret = gpio_get_raw_function(dev, offset, NULL);
	if (ret < 0)
		return ret;
	func = ret;
	len = snprintf(str, buffsize, "%s%d: %s",
		       priv->bank_name ? priv->bank_name : "",
		       offset, gpio_function[func]);
	if (func == GPIOF_INPUT || func == GPIOF_OUTPUT ||
	    func == GPIOF_UNUSED) {
		const char *label;
		bool used;

		ret = ops->get_value(dev, offset);
		if (ret < 0)
			return ret;
		used = gpio_get_function(dev, offset, &label) != GPIOF_UNUSED;
		snprintf(str + len, buffsize - len, ": %d [%c]%s%s",
			 ret,
			 used ? 'x' : ' ',
			 used ? " " : "",
			 label ? label : "");
	}

	return 0;
}

/*
 * get a number comprised of multiple GPIO values. gpio_num_array points to
 * the array of gpio pin numbers to scan, terminated by -1.
 */
unsigned gpio_get_values_as_int(const int *gpio_num_array)
{
	int gpio;
	unsigned bitmask = 1;
	unsigned vector = 0;

	while (bitmask &&
	       ((gpio = *gpio_num_array++) != -1)) {
		if (gpio_get_value(gpio))
			vector |= bitmask;
		bitmask <<= 1;
	}
	return vector;
}

/* We need to renumber the GPIOs when any driver is probed/removed */
static int gpio_renumber(struct udevice *removed_dev)
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
		if (device_active(dev) && dev != removed_dev) {
			uc_priv = dev->uclass_priv;
			uc_priv->gpio_base = base;
			base += uc_priv->gpio_count;
		}
	}

	return 0;
}

static int gpio_post_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;

	uc_priv->name = calloc(uc_priv->gpio_count, sizeof(char *));
	if (!uc_priv->name)
		return -ENOMEM;

	return gpio_renumber(NULL);
}

static int gpio_pre_remove(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev->uclass_priv;
	int i;

	for (i = 0; i < uc_priv->gpio_count; i++) {
		if (uc_priv->name[i])
			free(uc_priv->name[i]);
	}
	free(uc_priv->name);

	return gpio_renumber(dev);
}

UCLASS_DRIVER(gpio) = {
	.id		= UCLASS_GPIO,
	.name		= "gpio",
	.post_probe	= gpio_post_probe,
	.pre_remove	= gpio_pre_remove,
	.per_device_auto_alloc_size = sizeof(struct gpio_dev_priv),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/uclass-internal.h>
#include <dt-bindings/gpio/gpio.h>
#include <errno.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <dm/device_compat.h>
#include <linux/bug.h>
#include <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/**
 * gpio_desc_init() - Initialize the GPIO descriptor
 *
 * @desc:	GPIO descriptor to initialize
 * @dev:	GPIO device
 * @offset:	Offset of device GPIO
 */
static void gpio_desc_init(struct gpio_desc *desc,
			   struct udevice *dev,
			   uint offset)
{
	desc->dev = dev;
	desc->offset = offset;
	desc->flags = 0;
}

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
		uc_priv = dev_get_uclass_priv(dev);
		if (gpio >= uc_priv->gpio_base &&
		    gpio < uc_priv->gpio_base + uc_priv->gpio_count) {
			gpio_desc_init(desc, dev, gpio - uc_priv->gpio_base);
			return 0;
		}
	}

	/* No such GPIO */
	return ret ? ret : -ENOENT;
}

int dm_gpio_lookup_name(const char *name, struct gpio_desc *desc)
{
	struct gpio_dev_priv *uc_priv = NULL;
	struct udevice *dev;
	ulong offset;
	int numeric;
	int ret;

	numeric = isdigit(*name) ? simple_strtoul(name, NULL, 10) : -1;
	for (ret = uclass_first_device(UCLASS_GPIO, &dev);
	     dev;
	     ret = uclass_next_device(&dev)) {
		int len;

		uc_priv = dev_get_uclass_priv(dev);
		if (numeric != -1) {
			offset = numeric - uc_priv->gpio_base;
			/* Allow GPIOs to be numbered from 0 */
			if (offset < uc_priv->gpio_count)
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

	gpio_desc_init(desc, dev, offset);

	return 0;
}

int gpio_lookup_name(const char *name, struct udevice **devp,
		     unsigned int *offsetp, unsigned int *gpiop)
{
	struct gpio_desc desc;
	int ret;

	if (devp)
		*devp = NULL;
	ret = dm_gpio_lookup_name(name, &desc);
	if (ret)
		return ret;

	if (devp)
		*devp = desc.dev;
	if (offsetp)
		*offsetp = desc.offset;
	if (gpiop) {
		struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(desc.dev);

		*gpiop = uc_priv->gpio_base + desc.offset;
	}

	return 0;
}

int gpio_xlate_offs_flags(struct udevice *dev, struct gpio_desc *desc,
			  struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -EINVAL;

	desc->offset = args->args[0];

	if (args->args_count < 2)
		return 0;

	desc->flags = 0;
	if (args->args[1] & GPIO_ACTIVE_LOW)
		desc->flags |= GPIOD_ACTIVE_LOW;

	/*
	 * need to test 2 bits for gpio output binding:
	 * OPEN_DRAIN (0x6) = SINGLE_ENDED (0x2) | LINE_OPEN_DRAIN (0x4)
	 * OPEN_SOURCE (0x2) = SINGLE_ENDED (0x2) | LINE_OPEN_SOURCE (0x0)
	 */
	if (args->args[1] & GPIO_SINGLE_ENDED) {
		if (args->args[1] & GPIO_LINE_OPEN_DRAIN)
			desc->flags |= GPIOD_OPEN_DRAIN;
		else
			desc->flags |= GPIOD_OPEN_SOURCE;
	}

	if (args->args[1] & GPIO_PULL_UP)
		desc->flags |= GPIOD_PULL_UP;

	if (args->args[1] & GPIO_PULL_DOWN)
		desc->flags |= GPIOD_PULL_DOWN;

	return 0;
}

static int gpio_find_and_xlate(struct gpio_desc *desc,
			       struct ofnode_phandle_args *args)
{
	struct dm_gpio_ops *ops = gpio_get_ops(desc->dev);

	if (ops->xlate)
		return ops->xlate(desc->dev, desc, args);
	else
		return gpio_xlate_offs_flags(desc->dev, desc, args);
}

#if defined(CONFIG_GPIO_HOG)

struct gpio_hog_priv {
	struct gpio_desc gpiod;
};

struct gpio_hog_data {
	int gpiod_flags;
	int value;
	u32 val[2];
};

static int gpio_hog_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_hog_data *plat = dev_get_platdata(dev);
	const char *nodename;
	int ret;

	plat->value = 0;
	if (dev_read_bool(dev, "input")) {
		plat->gpiod_flags = GPIOD_IS_IN;
	} else if (dev_read_bool(dev, "output-high")) {
		plat->value = 1;
		plat->gpiod_flags = GPIOD_IS_OUT;
	} else if (dev_read_bool(dev, "output-low")) {
		plat->gpiod_flags = GPIOD_IS_OUT;
	} else {
		printf("%s: missing gpio-hog state.\n", __func__);
		return -EINVAL;
	}
	ret = dev_read_u32_array(dev, "gpios", plat->val, 2);
	if (ret) {
		printf("%s: wrong gpios property, 2 values needed %d\n",
		       __func__, ret);
		return ret;
	}
	nodename = dev_read_string(dev, "line-name");
	if (nodename)
		device_set_name(dev, nodename);

	return 0;
}

static int gpio_hog_probe(struct udevice *dev)
{
	struct gpio_hog_data *plat = dev_get_platdata(dev);
	struct gpio_hog_priv *priv = dev_get_priv(dev);
	int ret;

	ret = gpio_dev_request_index(dev->parent, dev->name, "gpio-hog",
				     plat->val[0], plat->gpiod_flags,
				     plat->val[1], &priv->gpiod);
	if (ret < 0) {
		debug("%s: node %s could not get gpio.\n", __func__,
		      dev->name);
		return ret;
	}

	if (plat->gpiod_flags == GPIOD_IS_OUT) {
		ret = dm_gpio_set_value(&priv->gpiod, plat->value);
		if (ret < 0) {
			debug("%s: node %s could not set gpio.\n", __func__,
			      dev->name);
			return ret;
		}
	}

	return 0;
}

int gpio_hog_probe_all(void)
{
	struct udevice *dev;
	int ret;
	int retval = 0;

	for (uclass_first_device(UCLASS_NOP, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		if (dev->driver == DM_GET_DRIVER(gpio_hog)) {
			ret = device_probe(dev);
			if (ret) {
				printf("Failed to probe device %s err: %d\n",
				       dev->name, ret);
				retval = ret;
			}
		}
	}

	return retval;
}

int gpio_hog_lookup_name(const char *name, struct gpio_desc **desc)
{
	struct udevice *dev;

	*desc = NULL;
	gpio_hog_probe_all();
	if (!uclass_get_device_by_name(UCLASS_NOP, name, &dev)) {
		struct gpio_hog_priv *priv = dev_get_priv(dev);

		*desc = &priv->gpiod;
		return 0;
	}

	return -ENODEV;
}

U_BOOT_DRIVER(gpio_hog) = {
	.name	= "gpio_hog",
	.id	= UCLASS_NOP,
	.ofdata_to_platdata = gpio_hog_ofdata_to_platdata,
	.probe = gpio_hog_probe,
	.priv_auto_alloc_size = sizeof(struct gpio_hog_priv),
	.platdata_auto_alloc_size = sizeof(struct gpio_hog_data),
};
#else
int gpio_hog_lookup_name(const char *name, struct gpio_desc **desc)
{
	return 0;
}
#endif

int dm_gpio_request(struct gpio_desc *desc, const char *label)
{
	struct udevice *dev = desc->dev;
	struct gpio_dev_priv *uc_priv;
	char *str;
	int ret;

	uc_priv = dev_get_uclass_priv(dev);
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

static int dm_gpio_requestf(struct gpio_desc *desc, const char *fmt, ...)
{
#if !defined(CONFIG_SPL_BUILD) || !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
	va_list args;
	char buf[40];

	va_start(args, fmt);
	vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	return dm_gpio_request(desc, buf);
#else
	return dm_gpio_request(desc, fmt);
#endif
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
#if !defined(CONFIG_SPL_BUILD) || !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
	va_list args;
	char buf[40];

	va_start(args, fmt);
	vscnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	return gpio_request(gpio, buf);
#else
	return gpio_request(gpio, fmt);
#endif
}

int _dm_gpio_free(struct udevice *dev, uint offset)
{
	struct gpio_dev_priv *uc_priv;
	int ret;

	uc_priv = dev_get_uclass_priv(dev);
	if (!uc_priv->name[offset])
		return -ENXIO;
	if (gpio_get_ops(dev)->rfree) {
		ret = gpio_get_ops(dev)->rfree(dev, offset);
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

static int check_reserved(const struct gpio_desc *desc, const char *func)
{
	struct gpio_dev_priv *uc_priv;

	if (!dm_gpio_is_valid(desc))
		return -ENOENT;

	uc_priv = dev_get_uclass_priv(desc->dev);
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

static int _gpio_get_value(const struct gpio_desc *desc)
{
	int value;

	value = gpio_get_ops(desc->dev)->get_value(desc->dev, desc->offset);

	return desc->flags & GPIOD_ACTIVE_LOW ? !value : value;
}

int dm_gpio_get_value(const struct gpio_desc *desc)
{
	int ret;

	ret = check_reserved(desc, "get_value");
	if (ret)
		return ret;

	return _gpio_get_value(desc);
}

int dm_gpio_set_value(const struct gpio_desc *desc, int value)
{
	int ret;

	ret = check_reserved(desc, "set_value");
	if (ret)
		return ret;

	if (desc->flags & GPIOD_ACTIVE_LOW)
		value = !value;

	/*
	 * Emulate open drain by not actively driving the line high or
	 * Emulate open source by not actively driving the line low
	 */
	if ((desc->flags & GPIOD_OPEN_DRAIN && value) ||
	    (desc->flags & GPIOD_OPEN_SOURCE && !value))
		return gpio_get_ops(desc->dev)->direction_input(desc->dev,
								desc->offset);
	else if (desc->flags & GPIOD_OPEN_DRAIN ||
		 desc->flags & GPIOD_OPEN_SOURCE)
		return gpio_get_ops(desc->dev)->direction_output(desc->dev,
								desc->offset,
								value);

	gpio_get_ops(desc->dev)->set_value(desc->dev, desc->offset, value);
	return 0;
}

/* check dir flags invalid configuration */
static int check_dir_flags(ulong flags)
{
	if ((flags & GPIOD_IS_OUT) && (flags & GPIOD_IS_IN)) {
		log_debug("%s: flags 0x%lx has GPIOD_IS_OUT and GPIOD_IS_IN\n",
			  __func__, flags);
		return -EINVAL;
	}

	if ((flags & GPIOD_PULL_UP) && (flags & GPIOD_PULL_DOWN)) {
		log_debug("%s: flags 0x%lx has GPIOD_PULL_UP and GPIOD_PULL_DOWN\n",
			  __func__, flags);
		return -EINVAL;
	}

	if ((flags & GPIOD_OPEN_DRAIN) && (flags & GPIOD_OPEN_SOURCE)) {
		log_debug("%s: flags 0x%lx has GPIOD_OPEN_DRAIN and GPIOD_OPEN_SOURCE\n",
			  __func__, flags);
		return -EINVAL;
	}

	return 0;
}

static int _dm_gpio_set_dir_flags(struct gpio_desc *desc, ulong flags)
{
	struct udevice *dev = desc->dev;
	struct dm_gpio_ops *ops = gpio_get_ops(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int ret = 0;

	ret = check_dir_flags(flags);
	if (ret) {
		dev_dbg(dev,
			"%s error: set_dir_flags for gpio %s%d has invalid dir flags 0x%lx\n",
			desc->dev->name,
			uc_priv->bank_name ? uc_priv->bank_name : "",
			desc->offset, flags);

		return ret;
	}

	/* GPIOD_ are directly managed by driver in set_dir_flags*/
	if (ops->set_dir_flags) {
		ret = ops->set_dir_flags(dev, desc->offset, flags);
	} else {
		if (flags & GPIOD_IS_OUT) {
			ret = ops->direction_output(dev, desc->offset,
						    GPIOD_FLAGS_OUTPUT(flags));
		} else if (flags & GPIOD_IS_IN) {
			ret = ops->direction_input(dev, desc->offset);
		}
	}

	return ret;
}

int dm_gpio_set_dir_flags(struct gpio_desc *desc, ulong flags)
{
	int ret;

	ret = check_reserved(desc, "set_dir_flags");
	if (ret)
		return ret;

	/* combine the requested flags (for IN/OUT) and the descriptor flags */
	flags |= desc->flags;
	ret = _dm_gpio_set_dir_flags(desc, flags);

	/* update the descriptor flags */
	if (ret)
		desc->flags = flags;

	return ret;
}

int dm_gpio_set_dir(struct gpio_desc *desc)
{
	int ret;

	ret = check_reserved(desc, "set_dir");
	if (ret)
		return ret;

	return _dm_gpio_set_dir_flags(desc, desc->flags);
}

int dm_gpio_get_dir_flags(struct gpio_desc *desc, ulong *flags)
{
	struct udevice *dev = desc->dev;
	int ret, value;
	struct dm_gpio_ops *ops = gpio_get_ops(dev);
	ulong dir_flags;

	ret = check_reserved(desc, "get_dir_flags");
	if (ret)
		return ret;

	/* GPIOD_ are directly provided by driver except GPIOD_ACTIVE_LOW */
	if (ops->get_dir_flags) {
		ret = ops->get_dir_flags(dev, desc->offset, &dir_flags);
		if (ret)
			return ret;

		/* GPIOD_ACTIVE_LOW is saved in desc->flags */
		value = dir_flags & GPIOD_IS_OUT_ACTIVE ? 1 : 0;
		if (desc->flags & GPIOD_ACTIVE_LOW)
			value = !value;
		dir_flags &= ~(GPIOD_ACTIVE_LOW | GPIOD_IS_OUT_ACTIVE);
		dir_flags |= (desc->flags & GPIOD_ACTIVE_LOW);
		if (value)
			dir_flags |= GPIOD_IS_OUT_ACTIVE;
	} else {
		dir_flags = desc->flags;
		/* only GPIOD_IS_OUT_ACTIVE is provided by uclass */
		dir_flags &= ~GPIOD_IS_OUT_ACTIVE;
		if ((desc->flags & GPIOD_IS_OUT) && _gpio_get_value(desc))
			dir_flags |= GPIOD_IS_OUT_ACTIVE;
	}
	*flags = dir_flags;

	return 0;
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
	priv = dev_get_uclass_priv(dev);
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

static int get_function(struct udevice *dev, int offset, bool skip_unused,
			const char **namep)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
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
	priv = dev_get_uclass_priv(dev);
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

int gpio_claim_vector(const int *gpio_num_array, const char *fmt)
{
	int i, ret;
	int gpio;

	for (i = 0; i < 32; i++) {
		gpio = gpio_num_array[i];
		if (gpio == -1)
			break;
		ret = gpio_requestf(gpio, fmt, i);
		if (ret)
			goto err;
		ret = gpio_direction_input(gpio);
		if (ret) {
			gpio_free(gpio);
			goto err;
		}
	}

	return 0;
err:
	for (i--; i >= 0; i--)
		gpio_free(gpio_num_array[i]);

	return ret;
}

/*
 * get a number comprised of multiple GPIO values. gpio_num_array points to
 * the array of gpio pin numbers to scan, terminated by -1.
 */
int gpio_get_values_as_int(const int *gpio_list)
{
	int gpio;
	unsigned bitmask = 1;
	unsigned vector = 0;
	int ret;

	while (bitmask &&
	       ((gpio = *gpio_list++) != -1)) {
		ret = gpio_get_value(gpio);
		if (ret < 0)
			return ret;
		else if (ret)
			vector |= bitmask;
		bitmask <<= 1;
	}

	return vector;
}

int dm_gpio_get_values_as_int(const struct gpio_desc *desc_list, int count)
{
	unsigned bitmask = 1;
	unsigned vector = 0;
	int ret, i;

	for (i = 0; i < count; i++) {
		ret = dm_gpio_get_value(&desc_list[i]);
		if (ret < 0)
			return ret;
		else if (ret)
			vector |= bitmask;
		bitmask <<= 1;
	}

	return vector;
}

/**
 * gpio_request_tail: common work for requesting a gpio.
 *
 * ret:		return value from previous work in function which calls
 *		this function.
 *		This seems bogus (why calling this function instead not
 *		calling it and end caller function instead?).
 *		Because on error in caller function we want to set some
 *		default values in gpio desc and have a common error
 *		debug message, which provides this function.
 * nodename:	Name of node for which gpio gets requested
 *		used for gpio label name.
 * args:	pointer to output arguments structure
 * list_name:	Name of GPIO list
 *		used for gpio label name.
 * index:	gpio index in gpio list
 *		used for gpio label name.
 * desc:	pointer to gpio descriptor, filled from this
 *		function.
 * flags:	gpio flags to use.
 * add_index:	should index added to gpio label name
 * gpio_dev:	pointer to gpio device from which the gpio
 *		will be requested. If NULL try to get the
 *		gpio device with uclass_get_device_by_ofnode()
 *
 * return:	In error case this function sets default values in
 *		gpio descriptor, also emmits a debug message.
 *		On success it returns 0 else the error code from
 *		function calls, or the error code passed through
 *		ret to this function.
 *
 */
static int gpio_request_tail(int ret, const char *nodename,
			     struct ofnode_phandle_args *args,
			     const char *list_name, int index,
			     struct gpio_desc *desc, int flags,
			     bool add_index, struct udevice *gpio_dev)
{
	gpio_desc_init(desc, gpio_dev, 0);
	if (ret)
		goto err;

	if (!desc->dev) {
		ret = uclass_get_device_by_ofnode(UCLASS_GPIO, args->node,
						  &desc->dev);
		if (ret) {
			debug("%s: uclass_get_device_by_ofnode failed\n",
			      __func__);
			goto err;
		}
	}
	ret = gpio_find_and_xlate(desc, args);
	if (ret) {
		debug("%s: gpio_find_and_xlate failed\n", __func__);
		goto err;
	}
	ret = dm_gpio_requestf(desc, add_index ? "%s.%s%d" : "%s.%s",
			       nodename, list_name, index);
	if (ret) {
		debug("%s: dm_gpio_requestf failed\n", __func__);
		goto err;
	}
	ret = dm_gpio_set_dir_flags(desc, flags);
	if (ret) {
		debug("%s: dm_gpio_set_dir failed\n", __func__);
		goto err;
	}

	return 0;
err:
	debug("%s: Node '%s', property '%s', failed to request GPIO index %d: %d\n",
	      __func__, nodename, list_name, index, ret);
	return ret;
}

static int _gpio_request_by_name_nodev(ofnode node, const char *list_name,
				       int index, struct gpio_desc *desc,
				       int flags, bool add_index)
{
	struct ofnode_phandle_args args;
	int ret;

	ret = ofnode_parse_phandle_with_args(node, list_name, "#gpio-cells", 0,
					     index, &args);

	return gpio_request_tail(ret, ofnode_get_name(node), &args, list_name,
				 index, desc, flags, add_index, NULL);
}

int gpio_request_by_name_nodev(ofnode node, const char *list_name, int index,
			       struct gpio_desc *desc, int flags)
{
	return _gpio_request_by_name_nodev(node, list_name, index, desc, flags,
					   index > 0);
}

int gpio_request_by_name(struct udevice *dev, const char *list_name, int index,
			 struct gpio_desc *desc, int flags)
{
	struct ofnode_phandle_args args;
	ofnode node;
	int ret;

	ret = dev_read_phandle_with_args(dev, list_name, "#gpio-cells", 0,
					 index, &args);
	node = dev_ofnode(dev);
	return gpio_request_tail(ret, ofnode_get_name(node), &args, list_name,
				 index, desc, flags, index > 0, NULL);
}

int gpio_request_list_by_name_nodev(ofnode node, const char *list_name,
				    struct gpio_desc *desc, int max_count,
				    int flags)
{
	int count;
	int ret;

	for (count = 0; count < max_count; count++) {
		ret = _gpio_request_by_name_nodev(node, list_name, count,
						  &desc[count], flags, true);
		if (ret == -ENOENT)
			break;
		else if (ret)
			goto err;
	}

	/* We ran out of GPIOs in the list */
	return count;

err:
	gpio_free_list_nodev(desc, count - 1);

	return ret;
}

int gpio_request_list_by_name(struct udevice *dev, const char *list_name,
			      struct gpio_desc *desc, int max_count,
			      int flags)
{
	/*
	 * This isn't ideal since we don't use dev->name in the debug()
	 * calls in gpio_request_by_name(), but we can do this until
	 * gpio_request_list_by_name_nodev() can be dropped.
	 */
	return gpio_request_list_by_name_nodev(dev_ofnode(dev), list_name, desc,
					       max_count, flags);
}

int gpio_get_list_count(struct udevice *dev, const char *list_name)
{
	int ret;

	ret = fdtdec_parse_phandle_with_args(gd->fdt_blob, dev_of_offset(dev),
					     list_name, "#gpio-cells", 0, -1,
					     NULL);
	if (ret) {
		debug("%s: Node '%s', property '%s', GPIO count failed: %d\n",
		      __func__, dev->name, list_name, ret);
	}

	return ret;
}

int dm_gpio_free(struct udevice *dev, struct gpio_desc *desc)
{
	/* For now, we don't do any checking of dev */
	return _dm_gpio_free(desc->dev, desc->offset);
}

int gpio_free_list(struct udevice *dev, struct gpio_desc *desc, int count)
{
	int i;

	/* For now, we don't do any checking of dev */
	for (i = 0; i < count; i++)
		dm_gpio_free(dev, &desc[i]);

	return 0;
}

int gpio_free_list_nodev(struct gpio_desc *desc, int count)
{
	return gpio_free_list(NULL, desc, count);
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
			uc_priv = dev_get_uclass_priv(dev);
			uc_priv->gpio_base = base;
			base += uc_priv->gpio_count;
		}
	}

	return 0;
}

int gpio_get_number(const struct gpio_desc *desc)
{
	struct udevice *dev = desc->dev;
	struct gpio_dev_priv *uc_priv;

	if (!dev)
		return -1;
	uc_priv = dev->uclass_priv;

	return uc_priv->gpio_base + desc->offset;
}

static int gpio_post_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->name = calloc(uc_priv->gpio_count, sizeof(char *));
	if (!uc_priv->name)
		return -ENOMEM;

	return gpio_renumber(NULL);
}

static int gpio_pre_remove(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	int i;

	for (i = 0; i < uc_priv->gpio_count; i++) {
		if (uc_priv->name[i])
			free(uc_priv->name[i]);
	}
	free(uc_priv->name);

	return gpio_renumber(dev);
}

int gpio_dev_request_index(struct udevice *dev, const char *nodename,
			   char *list_name, int index, int flags,
			   int dtflags, struct gpio_desc *desc)
{
	struct ofnode_phandle_args args;

	args.node =  ofnode_null();
	args.args_count = 2;
	args.args[0] = index;
	args.args[1] = dtflags;

	return gpio_request_tail(0, nodename, &args, list_name, index, desc,
				 flags, 0, dev);
}

static int gpio_post_bind(struct udevice *dev)
{
	struct udevice *child;
	ofnode node;

#if defined(CONFIG_NEEDS_MANUAL_RELOC)
	struct dm_gpio_ops *ops = (struct dm_gpio_ops *)device_get_ops(dev);
	static int reloc_done;

	if (!reloc_done) {
		if (ops->request)
			ops->request += gd->reloc_off;
		if (ops->rfree)
			ops->rfree += gd->reloc_off;
		if (ops->direction_input)
			ops->direction_input += gd->reloc_off;
		if (ops->direction_output)
			ops->direction_output += gd->reloc_off;
		if (ops->get_value)
			ops->get_value += gd->reloc_off;
		if (ops->set_value)
			ops->set_value += gd->reloc_off;
		if (ops->get_function)
			ops->get_function += gd->reloc_off;
		if (ops->xlate)
			ops->xlate += gd->reloc_off;
		if (ops->set_dir_flags)
			ops->set_dir_flags += gd->reloc_off;
		if (ops->get_dir_flags)
			ops->get_dir_flags += gd->reloc_off;

		reloc_done++;
	}
#endif

	if (IS_ENABLED(CONFIG_GPIO_HOG)) {
		dev_for_each_subnode(node, dev) {
			if (ofnode_read_bool(node, "gpio-hog")) {
				const char *name = ofnode_get_name(node);
				int ret;

				ret = device_bind_driver_to_node(dev,
								 "gpio_hog",
								 name, node,
								 &child);
				if (ret)
					return ret;
			}
		}
	}
	return 0;
}

UCLASS_DRIVER(gpio) = {
	.id		= UCLASS_GPIO,
	.name		= "gpio",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_probe	= gpio_post_probe,
	.post_bind	= gpio_post_bind,
	.pre_remove	= gpio_pre_remove,
	.per_device_auto_alloc_size = sizeof(struct gpio_dev_priv),
};

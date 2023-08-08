/*
 * Take linux kernel driver drivers/i2c/busses/i2c-gpio.c for reference.
 *
 * Copyright (C) 2019 Marvell, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* This driver handles generic i2c GPIO devices where a GPIO pin is indexed
 * by the register address and bit number.  This driver is based off of the
 * linux kernel gpio-i2c.c driver.
 *
 * Note that this driver should not be confused with i2c-gpio as this is
 * an i2c to gpio driver whereas the other driver is a gpio to i2c driver.
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <asm-generic/gpio.h>
#include <fdtdec.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dt-bindings/gpio/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

struct i2c_gpio_chip {
	int addr;	/** I2C address */
	int gpio_count;	/** No. of GPIOs supported by the chip */
	u8 *func;	/** Array to specify if pin is input or output */
};

static int i2c_gpio_write8(struct udevice *dev, uint offset, u8 byte)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);
	int ret;

	ret = dm_i2c_write(dev, offset, &byte, 1);
	if (ret)
		pr_err("%s(%s): i2c write failed to addr 0x%x\n",
		       __func__, dev->name, chip->chip_addr);

	return ret;
}

static int i2c_gpio_read8(struct udevice *dev, uint offset)
{
	struct dm_i2c_chip *chip = dev_get_parent_platdata(dev);
	u8 data;
	int ret;

	ret = dm_i2c_read(dev, offset, &data, 1);
	if (ret)
		pr_err("%s(%s): i2c read failed from addr 0x%x\n", __func__,
		       dev->name, chip->chip_addr);

	return ret < 0 ? ret : data;
}

static int i2c_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct i2c_gpio_chip *chip = dev_get_platdata(dev);
	int value;
	int byte = (offset >> 3);
	int bit = (offset & 7);

	if (offset >= chip->gpio_count) {
		pr_err("%s(%s): Error: offset %u out of range 0..%u\n",
		       __func__, dev->name, offset, chip->gpio_count);
		return -EINVAL;
	}
	value = i2c_gpio_read8(dev, byte);
	value = (value < 0) ? value : ((value >> bit) & 1);
	return value;
}

static int i2c_gpio_set_value(struct udevice *dev, unsigned int offset,
			      int value)
{
	struct i2c_gpio_chip *chip = dev_get_platdata(dev);
	unsigned int byte = offset >> 3;
	unsigned int bit = offset & 7;
	unsigned int mask = (1 << bit);
	int status;
	int was;

	if (offset >= chip->gpio_count) {
		pr_err("%s(%s): Error: offset %u out of range 0..%u\n",
		       __func__, dev->name, offset, chip->gpio_count);
		return -EINVAL;
	}

	was = i2c_gpio_get_value(dev, byte);
	if (was < 0)
		return was;

	if (value)
		was |= mask;
	else
		was &= ~mask;
	status = i2c_gpio_write8(dev, byte, was);
	return status;
}

static int i2c_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct i2c_gpio_chip *chip = dev_get_platdata(dev);
	int byte = offset >> 3;
	u8 bit = offset & 7;
	u8 mask = ~(1 << bit);

	chip->func[byte] &= mask;

	/* For open drain: set as input by letting output go high */

	return i2c_gpio_set_value(dev, offset, 1);
}

static int i2c_gpio_direction_output(struct udevice *dev, unsigned int offset,
				     int value)
{
	struct i2c_gpio_chip *chip = dev_get_platdata(dev);
	int byte = offset >> 3;
	u8 bit = offset & 7;
	u8 mask = (1 << bit);

	chip->func[byte] |= mask;

	return i2c_gpio_set_value(dev, offset, value);
}

static int i2c_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct i2c_gpio_chip *chip = dev_get_platdata(dev);
	int byte = offset >> 3;
	u8 bit = offset & 7;
	u8 mask = (1 << bit);

	return (chip->func[byte] & mask) ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int i2c_gpio_xlate(struct udevice *dev, struct gpio_desc *desc,
			  struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -EINVAL;

	desc->offset = args->args[0];
	desc->flags = 0;
	if (args->args_count > 1) {
		if (args->args[1] & GPIO_ACTIVE_LOW)
			desc->flags |= GPIOD_ACTIVE_LOW;
	}
	return 0;
}

static int i2c_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct i2c_gpio_chip *chip = dev_get_platdata(dev);
	char name[32], label[16], *str;
	const char *tmp;
	int size;
	const char *status;

	debug("%s(%s)\n", __func__, dev->name);

	status = ofnode_read_string(dev->node, "status");
	if (status && !strncmp(status, "ok", 2)) {
		debug("%s(%s): GPIO device disabled in device tree\n",
		      __func__, dev->name);
		return -ENODEV;
	}

	chip->addr = ofnode_read_u32_default(dev->node, "reg", 0);
	if (!chip->addr) {
		pr_err("%s(%s): Missing reg property\n",
		       __func__, dev->name);
		return -ENODEV;
	}
	chip->gpio_count = ofnode_read_u32_default(dev->node, "ngpios", 0);
	if (!chip->gpio_count) {
		pr_err("%s(%s): Missing ngpios property\n",
		       __func__, dev->name);
		return -ENODEV;
	}

	tmp = dev_read_prop(dev, "label", &size);
	if (tmp) {
		strlcpy(label, tmp, sizeof(label));
		snprintf(name, sizeof(name), "%s@%x_", label, chip->addr);
	} else {
		snprintf(name, sizeof(name), "gpio@%x_", chip->addr);
	}

	str = strdup(name);
	if (!str)
		return -ENOMEM;

	uc_priv->bank_name = str;
	uc_priv->gpio_count = chip->gpio_count;
	chip->func = calloc((chip->gpio_count + 7) / 8, 1);
	if (!chip->func)
		return -ENOMEM;

	debug("%s(%s): probed at address %d with %u gpios and name %s\n",
	      __func__, dev->name, chip->addr, chip->gpio_count, name);

	return 0;
}

static const struct dm_gpio_ops i2c_gpio_ops = {
	.direction_input	= i2c_gpio_direction_input,
	.direction_output	= i2c_gpio_direction_output,
	.get_value		= i2c_gpio_get_value,
	.set_value		= i2c_gpio_set_value,
	.get_function		= i2c_gpio_get_function,
	.xlate			= i2c_gpio_xlate,
};

static const struct udevice_id i2c_gpio_ids[] = {
	{ .compatible = "gpio-i2c" },
	{ }
};

U_BOOT_DRIVER(i2c_gpio) = {
	.name		= "gpio-i2c",
	.id		= UCLASS_GPIO,
	.ops		= &i2c_gpio_ops,
	.probe		= i2c_gpio_probe,
	.platdata_auto_alloc_size = sizeof(struct i2c_gpio_chip),
	.of_match	= i2c_gpio_ids,
};

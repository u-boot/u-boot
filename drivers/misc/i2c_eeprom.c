/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <dm.h>
#include <i2c.h>
#include <i2c_eeprom.h>

static int i2c_eeprom_read(struct udevice *dev, int offset, uint8_t *buf,
			   int size)
{
	return -ENODEV;
}

static int i2c_eeprom_write(struct udevice *dev, int offset,
			    const uint8_t *buf, int size)
{
	return -ENODEV;
}

struct i2c_eeprom_ops i2c_eeprom_std_ops = {
	.read	= i2c_eeprom_read,
	.write	= i2c_eeprom_write,
};

int i2c_eeprom_std_probe(struct udevice *dev)
{
	return 0;
}

static const struct udevice_id i2c_eeprom_std_ids[] = {
	{ .compatible = "i2c-eeprom" },
	{ }
};

U_BOOT_DRIVER(i2c_eeprom_std) = {
	.name		= "i2c_eeprom",
	.id		= UCLASS_I2C_EEPROM,
	.of_match	= i2c_eeprom_std_ids,
	.probe		= i2c_eeprom_std_probe,
	.priv_auto_alloc_size = sizeof(struct i2c_eeprom),
	.ops		= &i2c_eeprom_std_ops,
};

UCLASS_DRIVER(i2c_eeprom) = {
	.id		= UCLASS_I2C_EEPROM,
	.name		= "i2c_eeprom",
};

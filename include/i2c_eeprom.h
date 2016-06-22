/*
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __I2C_EEPROM
#define __I2C_EEPROM

struct i2c_eeprom_ops {
	int (*read)(struct udevice *dev, int offset, uint8_t *buf, int size);
	int (*write)(struct udevice *dev, int offset, const uint8_t *buf,
		     int size);
};

struct i2c_eeprom {
	/* The EEPROM's page size in byte */
	unsigned long pagesize;
	/* The EEPROM's page width in bits (pagesize = 2^pagewidth) */
	unsigned pagewidth;
};

#endif

/*
 * Test-related constants for sandbox
 *
 * Copyright (c) 2014 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_TEST_H
#define __ASM_TEST_H

/* The sandbox driver always permits an I2C device with this address */
#define SANDBOX_I2C_TEST_ADDR	0x59

enum sandbox_i2c_eeprom_test_mode {
	SIE_TEST_MODE_NONE,
	/* Permits read/write of only one byte per I2C transaction */
	SIE_TEST_MODE_SINGLE_BYTE,
};

void sandbox_i2c_eeprom_set_test_mode(struct udevice *dev,
				      enum sandbox_i2c_eeprom_test_mode mode);

void sandbox_i2c_eeprom_set_offset_len(struct udevice *dev, int offset_len);

#endif

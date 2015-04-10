/*
 * DS620 DTT support
 *
 * (C) Copyright 2014 3ADEV <http://www.3adev.com>
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Dallas Semiconductor's DS1621/1631 Digital Thermometer and Thermostat.
 */

#include <common.h>
#include <i2c.h>
#include <dtt.h>

/*
 * Device code
 */
#define DTT_I2C_DEV_CODE	0x48
#define DTT_START_CONVERT	0x51
#define DTT_TEMP		0xAA
#define DTT_CONFIG		0xAC

/*
 * Config register MSB bits
 */
#define DTT_CONFIG_1SHOT	0x01
#define DTT_CONFIG_AUTOC	0x02
#define DTT_CONFIG_R0		0x04 /* always 1 */
#define DTT_CONFIG_R1		0x08 /* always 1 */
#define DTT_CONFIG_TLF	0x10
#define DTT_CONFIG_THF	0x20
#define DTT_CONFIG_NVB	0x40
#define DTT_CONFIG_DONE	0x80

#define CHIP(sensor) (DTT_I2C_DEV_CODE + (sensor & 0x07))

int dtt_init_one(int sensor)
{
	uint8_t config = DTT_CONFIG_1SHOT
			| DTT_CONFIG_R0
			| DTT_CONFIG_R1;
	return i2c_write(CHIP(sensor), DTT_CONFIG, 1, &config, 1);
}

int dtt_get_temp(int sensor)
{
	uint8_t status;
	uint8_t temp[2];

	/* Start a conversion, may take up to 1 second. */
	i2c_write(CHIP(sensor), DTT_START_CONVERT, 1, NULL, 0);
	do {
		if (i2c_read(CHIP(sensor), DTT_CONFIG, 1, &status, 1))
			/* bail out if I2C error */
			status |= DTT_CONFIG_DONE;
	} while (!(status & DTT_CONFIG_DONE));
	if (i2c_read(CHIP(sensor), DTT_TEMP, 1, temp, 2))
		/* bail out if I2C error */
		return -274; /* below absolute zero == error */

	return ((int16_t)(temp[1] | (temp[0] << 8))) >> 7;
}

/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
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
#define DTT_I2C_DEV_CODE 	0x48	/* Dallas Semi's DS1621 */
#define DTT_READ_TEMP		0xAA
#define DTT_READ_COUNTER	0xA8
#define DTT_READ_SLOPE		0xA9
#define DTT_WRITE_START_CONV	0xEE
#define DTT_WRITE_STOP_CONV	0x22
#define DTT_TEMP_HIGH		0xA1
#define DTT_TEMP_LOW		0xA2
#define DTT_CONFIG		0xAC

/*
 * Config register bits
 */
#define DTT_CONFIG_1SHOT	0x01
#define DTT_CONFIG_POLARITY	0x02
#define DTT_CONFIG_R0		0x04	/* ds1631 only */
#define DTT_CONFIG_R1		0x08	/* ds1631 only */
#define DTT_CONFIG_NVB		0x10
#define DTT_CONFIG_TLF		0x20
#define DTT_CONFIG_THF		0x40
#define DTT_CONFIG_DONE		0x80


int dtt_read(int sensor, int reg)
{
	int dlen;
	uchar data[2];

	/* Calculate sensor address and command */
	sensor = DTT_I2C_DEV_CODE + (sensor & 0x07); /* Calculate addr of ds1621*/

	/* Prepare to handle 2 byte result */
	switch(reg) {
	case DTT_READ_TEMP:
	case DTT_TEMP_HIGH:
	case DTT_TEMP_LOW:
		dlen = 2;
		break;
	default:
		dlen = 1;
	}

	/* Now try to read the register */
	if (i2c_read(sensor, reg, 1, data, dlen) != 0)
		return 1;

	/* Handle 2 byte result */
	if (dlen == 2)
		return (short)((data[0] << 8) | data[1]);

	return (int)data[0];
}


int dtt_write(int sensor, int reg, int val)
{
	int dlen;
	uchar data[2];

	/* Calculate sensor address and register */
	sensor = DTT_I2C_DEV_CODE + (sensor & 0x07);

	/* Handle various data sizes. */
	switch(reg) {
	case DTT_READ_TEMP:
	case DTT_TEMP_HIGH:
	case DTT_TEMP_LOW:
		dlen = 2;
		data[0] = (char)((val >> 8) & 0xff);	/* MSB first */
		data[1] = (char)(val & 0xff);
		break;
	case DTT_WRITE_START_CONV:
	case DTT_WRITE_STOP_CONV:
		dlen = 0;
		data[0] = (char)0;
		data[1] = (char)0;
		break;
	default:
		dlen = 1;
		data[0] = (char)(val & 0xff);
	}

	/* Write value to device */
	if (i2c_write(sensor, reg, 1, data, dlen) != 0)
		return 1;

	/* Poll NV memory busy bit in case write was to register stored in EEPROM */
	while(i2c_reg_read(sensor, DTT_CONFIG) & DTT_CONFIG_NVB)
		;

	return 0;
}


int dtt_init_one(int sensor)
{
	int val;

	/* Setup High Temp */
	val = ((CONFIG_SYS_DTT_MAX_TEMP * 2) << 7) & 0xff80;
	if (dtt_write(sensor, DTT_TEMP_HIGH, val) != 0)
		return 1;

	/* Setup Low Temp - hysteresis */
	val = (((CONFIG_SYS_DTT_MAX_TEMP - CONFIG_SYS_DTT_HYSTERESIS) * 2) << 7) & 0xff80;
	if (dtt_write(sensor, DTT_TEMP_LOW, val) != 0)
		return 1;

	/*
	 * Setup configuraton register
	 *
	 * Clear THF & TLF, Reserved = 1, Polarity = Active Low, One Shot = YES
	 *
	 * We run in polled mode, since there isn't any way to know if this
	 * lousy device is ready to provide temperature readings on power up.
	 */
	val = 0x9;
	if (dtt_write(sensor, DTT_CONFIG, val) != 0)
		return 1;

	return 0;
}

int dtt_get_temp(int sensor)
{
	int i;

	/* Start a conversion, may take up to 1 second. */
	dtt_write(sensor, DTT_WRITE_START_CONV, 0);
	for (i = 0; i <= 10; i++) {
		udelay(100000);
		if (dtt_read(sensor, DTT_CONFIG) & DTT_CONFIG_DONE)
			break;
	}

	return (dtt_read(sensor, DTT_READ_TEMP) / 256);
}

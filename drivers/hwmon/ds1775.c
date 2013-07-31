/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Dallas Semiconductor's DS1775 Digital Thermometer and Thermostat
 */

#include <common.h>

#include <i2c.h>
#include <dtt.h>

#define DTT_I2C_DEV_CODE	CONFIG_SYS_I2C_DTT_ADDR /* Dallas Semi's DS1775 device code */
#define DTT_READ_TEMP		0x0
#define DTT_CONFIG		0x1
#define DTT_TEMP_HYST		0x2
#define DTT_TEMP_OS		0x3

int dtt_read(int sensor, int reg)
{
	int dlen;
	uchar data[2];

	/*
	 * Calculate sensor address and command
	 */
	sensor = DTT_I2C_DEV_CODE + (sensor & 0x07); /* Calculate addr of ds1775 */

	/*
	 * Prepare to handle 2 byte result
	 */
	if ((reg == DTT_READ_TEMP) ||
	    (reg == DTT_TEMP_OS) || (reg == DTT_TEMP_HYST))
		dlen = 2;
	else
		dlen = 1;

	/*
	 * Now try to read the register
	 */
	if (i2c_read(sensor, reg, 1, data, dlen) != 0)
		return 1;

	/*
	 * Handle 2 byte result
	 */
	if (dlen == 2)
		return ((int)((short)data[1] + (((short)data[0]) << 8)));

	return (int) data[0];
}


int dtt_write(int sensor, int reg, int val)
{
	int dlen;
	uchar data[2];

	/*
	 * Calculate sensor address and register
	 */
	sensor = DTT_I2C_DEV_CODE + (sensor & 0x07);

	/*
	 * Handle various data sizes
	 */
	if ((reg == DTT_READ_TEMP) ||
	    (reg == DTT_TEMP_OS) || (reg == DTT_TEMP_HYST)) {
		dlen = 2;
		data[0] = (char)((val >> 8) & 0xff); /* MSB first */
		data[1] = (char)(val & 0xff);
	} else {
		dlen = 1;
		data[0] = (char)(val & 0xff);
	}

	/*
	 * Write value to device
	 */
	if (i2c_write(sensor, reg, 1, data, dlen) != 0)
		return 1;

	return 0;
}


int dtt_init_one(int sensor)
{
	int val;

	/*
	 * Setup High Temp
	 */
	val = ((CONFIG_SYS_DTT_MAX_TEMP * 2) << 7) & 0xff80;
	if (dtt_write(sensor, DTT_TEMP_OS, val) != 0)
		return 1;
	udelay(50000);			/* Max 50ms */

	/*
	 * Setup Low Temp - hysteresis
	 */
	val = (((CONFIG_SYS_DTT_MAX_TEMP - CONFIG_SYS_DTT_HYSTERESIS) * 2) << 7) & 0xff80;
	if (dtt_write(sensor, DTT_TEMP_HYST, val) != 0)
		return 1;
	udelay(50000);			/* Max 50ms */

	/*
	 * Setup configuraton register
	 *
	 * Fault Tolerance limits 4, Thermometer resolution bits is 9,
	 * Polarity = Active Low,continuous conversion mode, Thermostat
	 * mode is interrupt mode
	 */
	val = 0xa;
	if (dtt_write(sensor, DTT_CONFIG, val) != 0)
		return 1;
	udelay(50000);			/* Max 50ms */

	return 0;
}

int dtt_get_temp(int sensor)
{
	return (dtt_read(sensor, DTT_READ_TEMP) / 256);
}

/*
 * (C) Copyright 2007-2008
 * Dirk Eibach,  Guntermann & Drunck GmbH, eibach@gdsys.de
 * based on lm75.c by Bill Hunter
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * National LM63 Temperature Sensor
 */

#include <common.h>
#include <i2c.h>
#include <dtt.h>

#define DTT_I2C_DEV_CODE 0x4C	/* National LM63 device */

#define DTT_READ_TEMP_RMT_MSB	0x01
#define DTT_CONFIG		0x03
#define DTT_READ_TEMP_RMT_LSB	0x10
#define DTT_TACHLIM_LSB		0x48
#define DTT_TACHLIM_MSB		0x49
#define DTT_FAN_CONFIG		0x4A
#define DTT_PWM_FREQ		0x4D
#define DTT_PWM_LOOKUP_BASE	0x50

struct pwm_lookup_entry {
	u8 temp;
	u8 pwm;
};

/*
 * Device code
 */

int dtt_read(int sensor, int reg)
{
	int dlen;
	uchar data[2];

	/*
	 * Calculate sensor address and register.
	 */
	sensor = DTT_I2C_DEV_CODE;	/* address of lm63 is not adjustable */

	dlen = 1;

	/*
	 * Now try to read the register.
	 */
	if (i2c_read(sensor, reg, 1, data, dlen) != 0)
		return -1;

	return (int)data[0];
}				/* dtt_read() */

int dtt_write(int sensor, int reg, int val)
{
	int dlen;
	uchar data[2];

	/*
	 * Calculate sensor address and register.
	 */
	sensor = DTT_I2C_DEV_CODE;	/* address of lm63 is not adjustable */

	dlen = 1;
	data[0] = (char)(val & 0xff);

	/*
	 * Write value to register.
	 */
	if (i2c_write(sensor, reg, 1, data, dlen) != 0)
		return 1;

	return 0;
}				/* dtt_write() */

static int _dtt_init(int sensor)
{
	int i;
	int val;

	struct pwm_lookup_entry pwm_lookup[] = CONFIG_DTT_PWM_LOOKUPTABLE;

	/*
	 * Set PWM Frequency to 2.5% resolution
	 */
	val = 20;
	if (dtt_write(sensor, DTT_PWM_FREQ, val) != 0)
		return 1;

	/*
	 * Set Tachometer Limit
	 */
	val = CONFIG_DTT_TACH_LIMIT;
	if (dtt_write(sensor, DTT_TACHLIM_LSB, val & 0xff) != 0)
		return 1;
	if (dtt_write(sensor, DTT_TACHLIM_MSB, (val >> 8) & 0xff) != 0)
		return 1;

	/*
	 * Setup PWM Lookup-Table
	 */
	for (i = 0; i < sizeof(pwm_lookup) / sizeof(struct pwm_lookup_entry);
	     i++) {
		int address = DTT_PWM_LOOKUP_BASE + 2 * i;
		val = pwm_lookup[i].temp;
		if (dtt_write(sensor, address, val) != 0)
			return 1;
		val = pwm_lookup[i].pwm;
		if (dtt_write(sensor, address + 1, val) != 0)
			return 1;
	}

	/*
	 * Enable PWM Lookup-Table, PWM Clock 360 kHz, Tachometer Mode 2
	 */
	val = 0x02;
	if (dtt_write(sensor, DTT_FAN_CONFIG, val) != 0)
		return 1;

	/*
	 * Enable Tach input
	 */
	val = dtt_read(sensor, DTT_CONFIG) | 0x04;
	if (dtt_write(sensor, DTT_CONFIG, val) != 0)
		return 1;

	return 0;
}

int dtt_get_temp(int sensor)
{
	s16 temp = (dtt_read(sensor, DTT_READ_TEMP_RMT_MSB) << 8)
	    | (dtt_read(sensor, DTT_READ_TEMP_RMT_LSB));

	/* Ignore LSB for now, U-Boot only prints natural numbers */
	return temp >> 8;
}

int dtt_init(void)
{
	int i;
	unsigned char sensors[] = CONFIG_DTT_SENSORS;
	const char *const header = "DTT:   ";

	for (i = 0; i < sizeof(sensors); i++) {
		if (_dtt_init(sensors[i]) != 0)
			printf("%s%d FAILED INIT\n", header, i + 1);
		else
			printf("%s%d is %i C\n", header, i + 1,
			       dtt_get_temp(sensors[i]));
	}

	return 0;
}

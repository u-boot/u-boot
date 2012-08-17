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
 * National LM63/LM64 Temperature Sensor
 * Main difference: LM 64 has -16 Kelvin temperature offset
 */

#include <common.h>
#include <i2c.h>
#include <dtt.h>

#define DTT_I2C_LM63_ADDR	0x4C	/* National LM63 device */

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
	if (!sensor)
		sensor = DTT_I2C_LM63_ADDR;	/* legacy config */

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
	if (!sensor)
		sensor = DTT_I2C_LM63_ADDR;	/* legacy config */

	dlen = 1;
	data[0] = (char)(val & 0xff);

	/*
	 * Write value to register.
	 */
	if (i2c_write(sensor, reg, 1, data, dlen) != 0)
		return 1;

	return 0;
}				/* dtt_write() */

static int is_lm64(int sensor)
{
	return sensor && (sensor != DTT_I2C_LM63_ADDR);
}

int dtt_init_one(int sensor)
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
	 * Make sure PWM Lookup-Table is writeable
	 */
	if (dtt_write(sensor, DTT_FAN_CONFIG, 0x20) != 0)
		return 1;

	/*
	 * Setup PWM Lookup-Table
	 */
	for (i = 0; i < sizeof(pwm_lookup) / sizeof(struct pwm_lookup_entry);
	     i++) {
		int address = DTT_PWM_LOOKUP_BASE + 2 * i;
		val = pwm_lookup[i].temp;
		if (is_lm64(sensor))
			val -= 16;
		if (dtt_write(sensor, address, val) != 0)
			return 1;
		val = dtt_read(sensor, address);
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

	if (is_lm64(sensor))
		temp += 16 << 8;

	/* Ignore LSB for now, U-Boot only prints natural numbers */
	return temp >> 8;
}

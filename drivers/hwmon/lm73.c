/*
 * (C) Copyright 2007-2008
 * Larry Johnson, lrj@acm.org
 *
 * based on dtt/lm75.c which is ...
 *
 * (C) Copyright 2001
 * Bill Hunter,  Wave 7 Optics, williamhunter@mediaone.net
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
 * National Semiconductor LM73 Temperature Sensor
 */

#include <common.h>
#include <i2c.h>
#include <dtt.h>

/*
 * Device code
 */
#define DTT_I2C_DEV_CODE 0x48	/* National Semi's LM73 device */

int dtt_read(int const sensor, int const reg)
{
	int dlen;
	uint8_t data[2];

	/*
	 * Validate 'reg' param and get register size.
	 */
	switch (reg) {
	case DTT_CONFIG:
	case DTT_CONTROL:
		dlen = 1;
		break;
	case DTT_READ_TEMP:
	case DTT_TEMP_HIGH:
	case DTT_TEMP_LOW:
	case DTT_ID:
		dlen = 2;
		break;
	default:
		return -1;
	}
	/*
	 * Try to read the register at the calculated sensor address.
	 */
	if (0 !=
	    i2c_read(DTT_I2C_DEV_CODE + (sensor & 0x07), reg, 1, data, dlen))
		return -1;
	/*
	 * Handle 2 byte result.
	 */
	if (2 == dlen)
		return (int)((unsigned)data[0] << 8 | (unsigned)data[1]);

	return (int)data[0];
} /* dtt_read() */

int dtt_write(int const sensor, int const reg, int const val)
{
	int dlen;
	uint8_t data[2];

	/*
	 * Validate 'reg' param and handle register size
	 */
	switch (reg) {
	case DTT_CONFIG:
	case DTT_CONTROL:
		dlen = 1;
		data[0] = (uint8_t) val;
		break;
	case DTT_TEMP_HIGH:
	case DTT_TEMP_LOW:
		dlen = 2;
		data[0] = (uint8_t) (val >> 8);	/* MSB first */
		data[1] = (uint8_t) val;
		break;
	default:
		return -1;
	}
	/*
	 * Write value to register at the calculated sensor address.
	 */
	return 0 != i2c_write(DTT_I2C_DEV_CODE + (sensor & 0x07), reg, 1, data,
			      dlen);
} /* dtt_write() */

static int _dtt_init(int const sensor)
{
	int val;

	/*
	 * Validate the Identification register
	 */
	if (0x0190 != dtt_read(sensor, DTT_ID))
		return -1;
	/*
	 * Setup THIGH (upper-limit) and TLOW (lower-limit) registers
	 */
	val = CFG_DTT_MAX_TEMP << 7;
	if (dtt_write(sensor, DTT_TEMP_HIGH, val))
		return -1;

	val = CFG_DTT_MIN_TEMP << 7;
	if (dtt_write(sensor, DTT_TEMP_LOW, val))
		return -1;
	/*
	 * Setup configuraton register
	 */
	/* config = alert active low, disabled, and reset */
	val = 0x64;
	if (dtt_write(sensor, DTT_CONFIG, val))
		return -1;
	/*
	 * Setup control/status register
	 */
	/* control = temp resolution 0.25C */
	val = 0x00;
	if (dtt_write(sensor, DTT_CONTROL, val))
		return -1;

	dtt_read(sensor, DTT_CONTROL);	/* clear temperature flags */
	return 0;
} /* _dtt_init() */

int dtt_init(void)
{
	int i;
	unsigned char sensors[] = CONFIG_DTT_SENSORS;
	const char *const header = "DTT:   ";

	for (i = 0; i < sizeof(sensors); i++) {
		if (0 != _dtt_init(sensors[i]))
			printf("%s%d FAILED INIT\n", header, i + 1);
		else
			printf("%s%d is %i C\n", header, i + 1,
			       dtt_get_temp(sensors[i]));
	}
	return 0;
} /* dtt_init() */

int dtt_get_temp(int const sensor)
{
	int const ret = dtt_read(sensor, DTT_READ_TEMP);

	if (ret < 0) {
		printf("DTT temperature read failed.\n");
		return 0;
	}
	return (int)((int16_t) ret + 0x0040) >> 7;
} /* dtt_get_temp() */

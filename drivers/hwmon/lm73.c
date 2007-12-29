/*
 * (C) Copyright 2007
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

#ifdef CONFIG_DTT_LM73
#if !defined(CFG_EEPROM_PAGE_WRITE_ENABLE) || \
	(CFG_EEPROM_PAGE_WRITE_BITS < 1)
# error "CFG_EEPROM_PAGE_WRITE_ENABLE must be defined and CFG_EEPROM_PAGE_WRITE_BITS must be greater than  1 to use CONFIG_DTT_LM73"
#endif

#include <i2c.h>
#include <dtt.h>

/*
 * Device code
 */
#define DTT_I2C_DEV_CODE 0x48	/* National Semi's LM73 device */

int dtt_read(int sensor, int reg)
{
	int dlen;
	uchar data[2];

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
	 * Calculate sensor address and register.
	 */
	sensor = DTT_I2C_DEV_CODE + (sensor & 0x07);	/* calculate LM73 addr */
	/*
	 * Now try to read the register.
	 */
	if (i2c_read(sensor, reg, 1, data, dlen) != 0)
		return -1;
	/*
	 * Handle 2 byte result.
	 */
	if (2 == dlen)
		return ((int)((short)data[1] + (((short)data[0]) << 8)));

	return (int)data[0];
} /* dtt_read() */

int dtt_write(int sensor, int reg, int val)
{
	int dlen;
	uchar data[2];

	/*
	 * Validate 'reg' param and handle register size
	 */
	switch (reg) {
	case DTT_CONFIG:
	case DTT_CONTROL:
		dlen = 1;
		data[0] = (char)(val & 0xff);
		break;
	case DTT_TEMP_HIGH:
	case DTT_TEMP_LOW:
		dlen = 2;
		data[0] = (char)((val >> 8) & 0xff);	/* MSB first */
		data[1] = (char)(val & 0xff);
		break;
	default:
		return -1;
	}
	/*
	 * Calculate sensor address and register.
	 */
	sensor = DTT_I2C_DEV_CODE + (sensor & 0x07);	/* calculate LM73 addr */
	/*
	 * Write value to register.
	 */
	return i2c_write(sensor, reg, 1, data, dlen) != 0;
} /* dtt_write() */

static int _dtt_init(int sensor)
{
	int val;

	/*
	 * Validate the Identification register
	 */
	if (0x0190 != dtt_read(sensor, DTT_ID))
		return 1;
	/*
	 * Setup THIGH (upper-limit) and TLOW (lower-limit) registers
	 */
	val = CFG_DTT_MAX_TEMP << 7;
	if (dtt_write(sensor, DTT_TEMP_HIGH, val))
		return 1;

	val = CFG_DTT_MIN_TEMP << 7;
	if (dtt_write(sensor, DTT_TEMP_LOW, val))
		return 1;
	/*
	 * Setup configuraton register
	 */
	/* config = alert active low, disabled, and reset */
	val = 0x64;
	if (dtt_write(sensor, DTT_CONFIG, val))
		return 1;
	/*
	 * Setup control/status register
	 */
	/* control = temp resolution 0.25C */
	val = 0x00;
	if (dtt_write(sensor, DTT_CONTROL, val))
		return 1;

	dtt_read(sensor, DTT_CONTROL);	/* clear temperature flags */
	return 0;
} /* _dtt_init() */

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
} /* dtt_init() */

int dtt_get_temp(int sensor)
{
	return (dtt_read(sensor, DTT_READ_TEMP) + 0x0040) >> 7;
} /* dtt_get_temp() */

#endif /* CONFIG_DTT_LM73 */

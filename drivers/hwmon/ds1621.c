/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
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
 * Dallas Semiconductor's DS1621 Digital Thermometer and Thermostat.
 */

#include <common.h>

#if !defined(CFG_EEPROM_PAGE_WRITE_ENABLE) || \
	(CFG_EEPROM_PAGE_WRITE_BITS < 1)
# error "CFG_EEPROM_PAGE_WRITE_ENABLE must be defined and CFG_EEPROM_PAGE_WRITE_BITS must be greater than 1 to use CONFIG_DTT_DS1621"
#endif
#include <i2c.h>
#include <dtt.h>

/*
 * Device code
 */
#define DTT_I2C_DEV_CODE 0x48			/* Dallas Semi's DS1621 */
#define DTT_READ_TEMP		0xAA
#define DTT_READ_COUNTER	0xA8
#define DTT_READ_SLOPE		0xA9
#define DTT_WRITE_START_CONV	0xEE
#define DTT_WRITE_STOP_CONV	0x22
#define DTT_TEMP_HIGH		0xA1
#define DTT_TEMP_LOW		0xA2
#define DTT_CONFIG		0xAC

int dtt_read(int sensor, int reg)
{
    int dlen;
    uchar data[2];

    /*
     * Calculate sensor address and command.
     *
     */
    sensor = DTT_I2C_DEV_CODE + (sensor & 0x07); /* Calculate addr of ds1621*/

    /*
     * Prepare to handle 2 byte result.
     */
    if ((reg == DTT_READ_TEMP) ||
	(reg == DTT_TEMP_HIGH) || (reg == DTT_TEMP_LOW))
	dlen = 2;
    else
	dlen = 1;

    /*
     * Now try to read the register.
     */
    if (i2c_read(sensor, reg, 1, data, dlen) != 0)
	return 1;

    /*
     * Handle 2 byte result.
     */
    if (dlen == 2)
	return ((int)((short)data[1] + (((short)data[0]) << 8)));

    return (int)data[0];
} /* dtt_read() */


int dtt_write(int sensor, int reg, int val)
{
    int dlen;
    uchar data[2];

    /*
     * Calculate sensor address and register.
     *
     */
    sensor = DTT_I2C_DEV_CODE + (sensor & 0x07);

    /*
     * Handle various data sizes.
     */
    if ((reg == DTT_READ_TEMP) ||
	(reg == DTT_TEMP_HIGH) || (reg == DTT_TEMP_LOW)) {
	dlen = 2;
	data[0] = (char)((val >> 8) & 0xff);	/* MSB first */
	data[1] = (char)(val & 0xff);
    }
    else if ((reg == DTT_WRITE_START_CONV) || (reg == DTT_WRITE_STOP_CONV)) {
	dlen = 0;
	data[0] = (char)0;
	data[1] = (char)0;
    }
    else {
	dlen = 1;
	data[0] = (char)(val & 0xff);
    }

    /*
     * Write value to device.
     */
    if (i2c_write(sensor, reg, 1, data, dlen) != 0)
	return 1;

    return 0;
} /* dtt_write() */


static int _dtt_init(int sensor)
{
    int val;

    /*
     * Setup High Temp.
     */
    val = ((CFG_DTT_MAX_TEMP * 2) << 7) & 0xff80;
    if (dtt_write(sensor, DTT_TEMP_HIGH, val) != 0)
	return 1;
    udelay(50000);				/* Max 50ms */

    /*
     * Setup Low Temp - hysteresis.
     */
    val = (((CFG_DTT_MAX_TEMP - CFG_DTT_HYSTERESIS) * 2) << 7) & 0xff80;
    if (dtt_write(sensor, DTT_TEMP_LOW, val) != 0)
	return 1;
    udelay(50000);				/* Max 50ms */

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
    udelay(50000);				/* Max 50ms */

    return 0;
} /* _dtt_init() */


int dtt_init (void)
{
    int i;
    unsigned char sensors[] = CONFIG_DTT_SENSORS;

    for (i = 0; i < sizeof(sensors); i++) {
	if (_dtt_init(sensors[i]) != 0)
	    printf("DTT%d:  FAILED\n", i+1);
	else
	    printf("DTT%d:  %i C\n", i+1, dtt_get_temp(sensors[i]));
    }

    return (0);
} /* dtt_init() */


int dtt_get_temp(int sensor)
{
    int i;

    /*
     * Start a conversion, may take up to 1 second.
     */
    dtt_write(sensor, DTT_WRITE_START_CONV, 0);
    for (i = 0; i <= 10; i++) {
	udelay(100000);
	if (dtt_read(sensor, DTT_CONFIG) & 0x80)
	    break;
    }

    return (dtt_read(sensor, DTT_READ_TEMP) / 256);
} /* dtt_get_temp() */

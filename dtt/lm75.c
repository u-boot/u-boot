/*
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
 * On Semiconductor's LM75 Temperature Sensor
 */

#include <common.h>

#ifdef CONFIG_DTT_LM75
#if !defined(CFG_EEPROM_PAGE_WRITE_ENABLE) || \
	(CFG_EEPROM_PAGE_WRITE_BITS < 1)
# error "CFG_EEPROM_PAGE_WRITE_ENABLE must be defined and CFG_EEPROM_PAGE_WRITE_BITS must be greater than  1 to use CONFIG_DTT_LM75"
#endif

#include <i2c.h>
#include <dtt.h>


/*
 * Device code
 */
#define DTT_I2C_DEV_CODE 0x48			/* ON Semi's LM75 device */

int dtt_read(int sensor, int reg)
{
    int dlen;
    uchar data[2];

    /*
     * Validate 'reg' param
     */
    if((reg < 0) || (reg > 3))
	return -1;

    /*
     * Calculate sensor address and register.
     */
    sensor = DTT_I2C_DEV_CODE + (sensor & 0x07); /* calculate address of lm75 */

    /*
     * Prepare to handle 2 byte result.
     */
    if ((reg == DTT_READ_TEMP) ||
	(reg == DTT_TEMP_HYST) ||
	(reg == DTT_TEMP_SET))
	dlen = 2;
    else
	dlen = 1;

    /*
     * Now try to read the register.
     */
    if (i2c_read(sensor, reg, 1, data, dlen) != 0)
	return -1;

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
     * Validate 'reg' param
     */
    if ((reg < 0) || (reg > 3))
	return 1;

    /*
     * Calculate sensor address and register.
     */
    sensor = DTT_I2C_DEV_CODE + (sensor & 0x07); /* calculate address of lm75 */

    /*
     * Handle 2 byte values.
     */
    if ((reg == DTT_READ_TEMP) ||
	(reg == DTT_TEMP_HYST) ||
	(reg == DTT_TEMP_SET)) {
	dlen = 2;
	data[0] = (char)((val >> 8) & 0xff);	/* MSB first */
	data[1] = (char)(val & 0xff);
    } else {
	dlen = 1;
	data[0] = (char)(val & 0xff);
    }

    /*
     * Write value to register.
     */
    if (i2c_write(sensor, reg, 1, data, dlen) != 0)
	return 1;

    return 0;
} /* dtt_write() */


static int _dtt_init(int sensor)
{
    int val;

    /*
     * Setup TSET ( trip point ) register
     */
    val = ((CFG_DTT_MAX_TEMP * 2) << 7) & 0xff80; /* trip */
    if (dtt_write(sensor, DTT_TEMP_SET, val) != 0)
	return 1;

    /*
     * Setup THYST ( untrip point ) register - Hysteresis
     */
    val = (((CFG_DTT_MAX_TEMP - CFG_DTT_HYSTERESIS) * 2) << 7) & 0xff80;
    if (dtt_write(sensor, DTT_TEMP_HYST, val) != 0)
	return 1;

    /*
     * Setup configuraton register
     */
#ifdef CONFIG_DTT_AD7414
    /* config = alert active low and disabled */
    val = 0x60;
#else
    /* config = 6 sample integration, int mode, active low, and enable */
    val = 0x18;
#endif
    if (dtt_write(sensor, DTT_CONFIG, val) != 0)
	return 1;

    return 0;
} /* _dtt_init() */


int dtt_init (void)
{
    int i;
    unsigned char sensors[] = CONFIG_DTT_SENSORS;
    const char *const header = "DTT:   ";

    for (i = 0; i < sizeof(sensors); i++) {
	if (_dtt_init(sensors[i]) != 0)
	    printf("%s%d FAILED INIT\n", header, i+1);
	else
	    printf("%s%d is %i C\n", header, i+1,
		   dtt_get_temp(sensors[i]));
    }

    return (0);
} /* dtt_init() */

int dtt_get_temp(int sensor)
{
    return (dtt_read(sensor, DTT_READ_TEMP) / 256);
} /* dtt_get_temp() */

#endif /* CONFIG_DTT_LM75 */

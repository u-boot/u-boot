/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Enginnering <hs@denx.de>
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
 * On Semiconductor's LM81 Temperature Sensor
 */

#include <common.h>

#if !defined(CFG_EEPROM_PAGE_WRITE_ENABLE) || \
	(CFG_EEPROM_PAGE_WRITE_BITS < 1)
# error "CFG_EEPROM_PAGE_WRITE_ENABLE must be defined and CFG_EEPROM_PAGE_WRITE_BITS must be greater than  1 to use CONFIG_DTT_LM81"
#endif

#include <i2c.h>
#include <dtt.h>

/*
 * Device code
 */
#define DTT_I2C_DEV_CODE 0x2c			/* ON Semi's LM81 device */
#define DTT_READ_TEMP		0x27
#define DTT_CONFIG_TEMP		0x4b
#define DTT_TEMP_MAX		0x39
#define DTT_TEMP_HYST		0x3a
#define DTT_CONFIG		0x40

int dtt_read(int sensor, int reg)
{
    int dlen = 1;
    uchar data[2];

    /*
     * Calculate sensor address and register.
     */
    sensor = DTT_I2C_DEV_CODE + (sensor & 0x03); /* calculate address of lm81 */

    /*
     * Now try to read the register.
     */
    if (i2c_read(sensor, reg, 1, data, dlen) != 0)
	return -1;

    return (int)data[0];
} /* dtt_read() */


int dtt_write(int sensor, int reg, int val)
{
    uchar data;

    /*
     * Calculate sensor address and register.
     */
    sensor = DTT_I2C_DEV_CODE + (sensor & 0x03); /* calculate address of lm81 */

    data = (char)(val & 0xff);

    /*
     * Write value to register.
     */
    if (i2c_write(sensor, reg, 1, &data, 1) != 0)
	return 1;

    return 0;
} /* dtt_write() */

#define DTT_MANU	0x3e
#define DTT_REV		0x3f
#define DTT_CONFIG	0x40
#define DTT_ADR		0x48

static int _dtt_init(int sensor)
{
	int	man;
	int	adr;
	int	rev;

	if (dtt_write (sensor, DTT_CONFIG, 0x01) < 0)
		return 1;
	/* The LM81 needs 400ms to get the correct values ... */
	udelay (400000);
	man = dtt_read (sensor, DTT_MANU);
	if (man != 0x01)
		return 1;
	adr = dtt_read (sensor, DTT_ADR);
	if (adr < 0)
		return 1;
	rev = dtt_read (sensor, DTT_REV);
	if (adr < 0)
		return 1;

	printf ("DTT:   Found LM81@%x Rev: %d\n", adr, rev);
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

#define TEMP_FROM_REG(temp) \
   ((temp)<256?((((temp)&0x1fe) >> 1) * 10)	 + ((temp) & 1) * 5:  \
	       ((((temp)&0x1fe) >> 1) -255) * 10 - ((temp) & 1) * 5)  \

int dtt_get_temp(int sensor)
{
	int val = dtt_read (sensor, DTT_READ_TEMP);
	int tmpcnf = dtt_read (sensor, DTT_CONFIG_TEMP);

	return (TEMP_FROM_REG((val << 1) + ((tmpcnf & 0x80) >> 7))) / 10;
} /* dtt_get_temp() */

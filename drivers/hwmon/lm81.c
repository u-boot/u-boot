/*
 * (C) Copyright 2006
 * Heiko Schocher, DENX Software Enginnering <hs@denx.de>
 *
 * based on dtt/lm75.c which is ...
 *
 * (C) Copyright 2001
 * Bill Hunter,  Wave 7 Optics, williamhunter@mediaone.net
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * On Semiconductor's LM81 Temperature Sensor
 */

#include <common.h>
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

int dtt_init_one(int sensor)
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
	if (rev < 0)
		return 1;

	debug ("DTT:   Found LM81@%x Rev: %d\n", adr, rev);
	return 0;
} /* dtt_init_one() */


#define TEMP_FROM_REG(temp) \
   ((temp)<256?((((temp)&0x1fe) >> 1) * 10)	 + ((temp) & 1) * 5:  \
	       ((((temp)&0x1fe) >> 1) -255) * 10 - ((temp) & 1) * 5)  \

int dtt_get_temp(int sensor)
{
	int val = dtt_read (sensor, DTT_READ_TEMP);
	int tmpcnf = dtt_read (sensor, DTT_CONFIG_TEMP);

	return (TEMP_FROM_REG((val << 1) + ((tmpcnf & 0x80) >> 7))) / 10;
} /* dtt_get_temp() */

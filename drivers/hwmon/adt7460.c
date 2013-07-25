/*
 * (C) Copyright 2008
 * Ricado Ribalda-Universidad Autonoma de Madrid, ricardo.ribalda@uam.es
 * This work has been supported by: QTechnology  http://qtec.com/
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <dtt.h>

#define ADT7460_ADDRESS		0x2c
#define ADT7460_INVALID		128
#define ADT7460_CONFIG		0x40
#define ADT7460_REM1_TEMP	0x25
#define ADT7460_LOCAL_TEMP	0x26
#define ADT7460_REM2_TEMP	0x27

int dtt_read(int sensor, int reg)
{
	u8 dir = reg;
	u8 data;

	if (i2c_read(ADT7460_ADDRESS, dir, 1, &data, 1) == -1)
		return -1;
	if (data == ADT7460_INVALID)
		return -1;

	return data;
}

int dtt_write(int sensor, int reg, int val)
{
	u8 dir = reg;
	u8 data = val;

	if (i2c_write(ADT7460_ADDRESS, dir, 1, &data, 1) == -1)
		return -1;

	return 0;
}

int dtt_init_one(int sensor)
{
	printf("ADT7460 at I2C address 0x%2x\n", ADT7460_ADDRESS);

	if (dtt_write(0, ADT7460_CONFIG, 1) == -1) {
		puts("Error initialiting ADT7460\n");
		return -1;
	}

	return 0;
}

int dtt_get_temp(int sensor)
{
	int aux;
	u8 table[] =
	    { ADT7460_REM1_TEMP, ADT7460_LOCAL_TEMP, ADT7460_REM2_TEMP };

	if (sensor > 2) {
		puts("DTT sensor does not exist\n");
		return -1;
	}

	aux = dtt_read(0, table[sensor]);
	if (aux == -1) {
		puts("DTT temperature read failed\n");
		return -1;
	}

	return aux;
}

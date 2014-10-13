/*
 * (C) Copyright 2012
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <i2c.h>
#include <axp152.h>

enum axp152_reg {
	AXP152_CHIP_VERSION = 0x3,
	AXP152_DCDC2_VOLTAGE = 0x23,
	AXP152_DCDC3_VOLTAGE = 0x27,
	AXP152_DCDC4_VOLTAGE = 0x2B,
	AXP152_LDO2_VOLTAGE = 0x2A,
	AXP152_SHUTDOWN = 0x32,
};

#define AXP152_POWEROFF			(1 << 7)

static int axp152_write(enum axp152_reg reg, u8 val)
{
	return i2c_write(0x30, reg, 1, &val, 1);
}

static int axp152_read(enum axp152_reg reg, u8 *val)
{
	return i2c_read(0x30, reg, 1, val, 1);
}

static u8 axp152_mvolt_to_target(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp152_set_dcdc2(int mvolt)
{
	int rc;
	u8 current, target;

	target = axp152_mvolt_to_target(mvolt, 700, 2275, 25);

	/* Do we really need to be this gentle? It has built-in voltage slope */
	while ((rc = axp152_read(AXP152_DCDC2_VOLTAGE, &current)) == 0 &&
	       current != target) {
		if (current < target)
			current++;
		else
			current--;
		rc = axp152_write(AXP152_DCDC2_VOLTAGE, current);
		if (rc)
			break;
	}
	return rc;
}

int axp152_set_dcdc3(int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 50);

	return axp152_write(AXP152_DCDC3_VOLTAGE, target);
}

int axp152_set_dcdc4(int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 25);

	return axp152_write(AXP152_DCDC4_VOLTAGE, target);
}

int axp152_set_ldo2(int mvolt)
{
	u8 target = axp152_mvolt_to_target(mvolt, 700, 3500, 100);

	return axp152_write(AXP152_LDO2_VOLTAGE, target);
}

int axp152_init(void)
{
	u8 ver;
	int rc;

	rc = axp152_read(AXP152_CHIP_VERSION, &ver);
	if (rc)
		return rc;

	if (ver != 0x05)
		return -1;

	return 0;
}

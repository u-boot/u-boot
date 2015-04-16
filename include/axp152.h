/*
 * (C) Copyright 2012 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

enum axp152_reg {
	AXP152_CHIP_VERSION = 0x3,
	AXP152_DCDC2_VOLTAGE = 0x23,
	AXP152_DCDC3_VOLTAGE = 0x27,
	AXP152_DCDC4_VOLTAGE = 0x2B,
	AXP152_LDO2_VOLTAGE = 0x2A,
	AXP152_SHUTDOWN = 0x32,
};

#define AXP152_POWEROFF			(1 << 7)

int axp152_set_dcdc2(int mvolt);
int axp152_set_dcdc3(int mvolt);
int axp152_set_dcdc4(int mvolt);
int axp152_set_ldo2(int mvolt);
int axp152_init(void);

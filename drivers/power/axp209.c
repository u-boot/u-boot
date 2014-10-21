/*
 * (C) Copyright 2012
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <i2c.h>
#include <axp209.h>

enum axp209_reg {
	AXP209_POWER_STATUS = 0x00,
	AXP209_CHIP_VERSION = 0x03,
	AXP209_DCDC2_VOLTAGE = 0x23,
	AXP209_DCDC3_VOLTAGE = 0x27,
	AXP209_LDO24_VOLTAGE = 0x28,
	AXP209_LDO3_VOLTAGE = 0x29,
	AXP209_IRQ_STATUS5 = 0x4c,
	AXP209_SHUTDOWN = 0x32,
};

#define AXP209_POWER_STATUS_ON_BY_DC	(1 << 0)

#define AXP209_IRQ5_PEK_UP		(1 << 6)
#define AXP209_IRQ5_PEK_DOWN		(1 << 5)

#define AXP209_POWEROFF			(1 << 7)

static int axp209_write(enum axp209_reg reg, u8 val)
{
	return i2c_write(0x34, reg, 1, &val, 1);
}

static int axp209_read(enum axp209_reg reg, u8 *val)
{
	return i2c_read(0x34, reg, 1, val, 1);
}

static u8 axp209_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return (mvolt - min) / div;
}

int axp209_set_dcdc2(int mvolt)
{
	int rc;
	u8 cfg, current;

	cfg = axp209_mvolt_to_cfg(mvolt, 700, 2275, 25);

	/* Do we really need to be this gentle? It has built-in voltage slope */
	while ((rc = axp209_read(AXP209_DCDC2_VOLTAGE, &current)) == 0 &&
	       current != cfg) {
		if (current < cfg)
			current++;
		else
			current--;

		rc = axp209_write(AXP209_DCDC2_VOLTAGE, current);
		if (rc)
			break;
	}

	return rc;
}

int axp209_set_dcdc3(int mvolt)
{
	u8 cfg = axp209_mvolt_to_cfg(mvolt, 700, 3500, 25);

	return axp209_write(AXP209_DCDC3_VOLTAGE, cfg);
}

int axp209_set_ldo2(int mvolt)
{
	int rc;
	u8 cfg, reg;

	cfg = axp209_mvolt_to_cfg(mvolt, 1800, 3300, 100);

	rc = axp209_read(AXP209_LDO24_VOLTAGE, &reg);
	if (rc)
		return rc;

	/* LDO2 configuration is in upper 4 bits */
	reg = (reg & 0x0f) | (cfg << 4);
	return axp209_write(AXP209_LDO24_VOLTAGE, reg);
}

int axp209_set_ldo3(int mvolt)
{
	u8 cfg;

	if (mvolt == -1)
		cfg = 0x80;	/* determined by LDO3IN pin */
	else
		cfg = axp209_mvolt_to_cfg(mvolt, 700, 2275, 25);

	return axp209_write(AXP209_LDO3_VOLTAGE, cfg);
}

int axp209_set_ldo4(int mvolt)
{
	int rc;
	static const int vindex[] = {
		1250, 1300, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 2500,
		2700, 2800, 3000, 3100, 3200, 3300
	};
	u8 cfg, reg;

	/* Translate mvolt to register cfg value, requested <= selected */
	for (cfg = 15; vindex[cfg] > mvolt && cfg > 0; cfg--);

	rc = axp209_read(AXP209_LDO24_VOLTAGE, &reg);
	if (rc)
		return rc;

	/* LDO4 configuration is in lower 4 bits */
	reg = (reg & 0xf0) | (cfg << 0);
	return axp209_write(AXP209_LDO24_VOLTAGE, reg);
}

int axp209_init(void)
{
	u8 ver;
	int rc;

	rc = axp209_read(AXP209_CHIP_VERSION, &ver);
	if (rc)
		return rc;

	/* Low 4 bits is chip version */
	ver &= 0x0f;

	if (ver != 0x1)
		return -1;

	return 0;
}

int axp209_poweron_by_dc(void)
{
	u8 v;

	if (axp209_read(AXP209_POWER_STATUS, &v))
		return 0;

	return (v & AXP209_POWER_STATUS_ON_BY_DC);
}

int axp209_power_button(void)
{
	u8 v;

	if (axp209_read(AXP209_IRQ_STATUS5, &v))
		return 0;

	axp209_write(AXP209_IRQ_STATUS5, AXP209_IRQ5_PEK_DOWN);

	return v & AXP209_IRQ5_PEK_DOWN;
}

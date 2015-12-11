/*
 * AXP818 driver based on AXP221 driver
 *
 *
 * (C) Copyright 2015 Vishnu Patekar <vishnuptekar0510@gmail.com>
 *
 * Based on axp221.c
 * (C) Copyright 2014 Hans de Goede <hdegoede@redhat.com>
 * (C) Copyright 2013 Oliver Schinagl <oliver@schinagl.nl>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/gpio.h>
#include <asm/arch/pmic_bus.h>
#include <axp_pmic.h>

static u8 axp818_mvolt_to_cfg(int mvolt, int min, int max, int div)
{
	if (mvolt < min)
		mvolt = min;
	else if (mvolt > max)
		mvolt = max;

	return  (mvolt - min) / div;
}

int axp_set_dcdc1(unsigned int mvolt)
{
	int ret;
	u8 cfg = axp818_mvolt_to_cfg(mvolt, 1600, 3400, 100);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP818_OUTPUT_CTRL1,
					AXP818_OUTPUT_CTRL1_DCDC1_EN);

	ret = pmic_bus_write(AXP818_DCDC1_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP818_OUTPUT_CTRL1,
				AXP818_OUTPUT_CTRL1_DCDC1_EN);
}

int axp_set_dcdc2(unsigned int mvolt)
{
	int ret;
	u8 cfg;

	if (mvolt >= 1220)
		cfg = 70 + axp818_mvolt_to_cfg(mvolt, 1220, 1300, 20);
	else
		cfg = axp818_mvolt_to_cfg(mvolt, 500, 1200, 10);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP818_OUTPUT_CTRL1,
					AXP818_OUTPUT_CTRL1_DCDC2_EN);

	ret = pmic_bus_write(AXP818_DCDC2_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP818_OUTPUT_CTRL1,
				AXP818_OUTPUT_CTRL1_DCDC2_EN);
}

int axp_set_dcdc3(unsigned int mvolt)
{
	int ret;
	u8 cfg;

	if (mvolt >= 1220)
		cfg = 70 + axp818_mvolt_to_cfg(mvolt, 1220, 1300, 20);
	else
		cfg = axp818_mvolt_to_cfg(mvolt, 500, 1200, 10);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP818_OUTPUT_CTRL1,
					AXP818_OUTPUT_CTRL1_DCDC3_EN);

	ret = pmic_bus_write(AXP818_DCDC3_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP818_OUTPUT_CTRL1,
				AXP818_OUTPUT_CTRL1_DCDC3_EN);
}

int axp_set_dcdc5(unsigned int mvolt)
{
	int ret;
	u8 cfg;

	if (mvolt >= 1140)
		cfg = 32 + axp818_mvolt_to_cfg(mvolt, 1140, 1840, 20);
	else
		cfg = axp818_mvolt_to_cfg(mvolt, 800, 1120, 10);

	if (mvolt == 0)
		return pmic_bus_clrbits(AXP818_OUTPUT_CTRL1,
					AXP818_OUTPUT_CTRL1_DCDC5_EN);

	ret = pmic_bus_write(AXP818_DCDC5_CTRL, cfg);
	if (ret)
		return ret;

	return pmic_bus_setbits(AXP818_OUTPUT_CTRL1,
				AXP818_OUTPUT_CTRL1_DCDC5_EN);
}

int axp_init(void)
{
	u8 axp_chip_id;
	int ret;

	ret = pmic_bus_init();
	if (ret)
		return ret;

	ret = pmic_bus_read(AXP818_CHIP_ID, &axp_chip_id);
	if (ret)
		return ret;

	if (!(axp_chip_id == 0x51))
		return -ENODEV;
	else
		return ret;

	return 0;
}

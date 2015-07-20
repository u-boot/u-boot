/*
 * TWL4030 input
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <twl4030.h>

int twl4030_input_power_button(void)
{
	u8 data;

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_STS_HW_CONDITIONS, &data);

	if (data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_PWON)
		return 1;

	return 0;
}

int twl4030_input_charger(void)
{
	u8 data;

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_STS_HW_CONDITIONS, &data);

	if (data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_CHG)
		return 1;

	return 0;
}

int twl4030_input_usb(void)
{
	u8 data;

	twl4030_i2c_read_u8(TWL4030_CHIP_PM_MASTER,
			    TWL4030_PM_MASTER_STS_HW_CONDITIONS, &data);

	if (data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_USB ||
	    data & TWL4030_PM_MASTER_STS_HW_CONDITIONS_VBUS)
		return 1;

	return 0;
}

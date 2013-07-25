/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>
#ifdef CONFIG_TWL6030_POWER

#include <twl6030.h>

static int twl6030_gpadc_read_channel(u8 channel_no)
{
	u8 lsb = 0;
	u8 msb = 0;
	int ret = 0;

	ret = twl6030_i2c_read_u8(TWL6030_CHIP_ADC,
				  GPCH0_LSB + channel_no * 2, &lsb);
	if (ret)
		return ret;

	ret = twl6030_i2c_read_u8(TWL6030_CHIP_ADC,
				  GPCH0_MSB + channel_no * 2, &msb);
	if (ret)
		return ret;

	return (msb << 8) | lsb;
}

static int twl6030_gpadc_sw2_trigger(void)
{
	u8 val;
	int ret = 0;

	ret = twl6030_i2c_write_u8(TWL6030_CHIP_ADC, CTRL_P2, CTRL_P2_SP2);
	if (ret)
		return ret;

	/* Waiting until the SW1 conversion ends*/
	val =  CTRL_P2_BUSY;

	while (!((val & CTRL_P2_EOCP2) && (!(val & CTRL_P2_BUSY)))) {
		ret = twl6030_i2c_read_u8(TWL6030_CHIP_ADC, CTRL_P2, &val);
		if (ret)
			return ret;
		udelay(1000);
	}

	return 0;
}

void twl6030_stop_usb_charging(void)
{
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CONTROLLER_CTRL1, 0);

	return;
}

void twl6030_start_usb_charging(void)
{
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_VICHRG, CHARGERUSB_VICHRG_1500);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_CINLIMIT, CHARGERUSB_CIN_LIMIT_NONE);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CONTROLLER_INT_MASK, MBAT_TEMP);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_INT_MASK, MASK_MCHARGERUSB_THMREG);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_VOREG, CHARGERUSB_VOREG_4P0);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CHARGERUSB_CTRL2, CHARGERUSB_CTRL2_VITERM_400);
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, CHARGERUSB_CTRL1, TERM);
	/* Enable USB charging */
	twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER,
			     CONTROLLER_CTRL1, CONTROLLER_CTRL1_EN_CHARGER);
	return;
}

int twl6030_get_battery_current(void)
{
	int battery_current = 0;
	u8 msb = 0;
	u8 lsb = 0;

	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, FG_REG_11, &msb);
	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, FG_REG_10, &lsb);
	battery_current = ((msb << 8) | lsb);

	/* convert 10 bit signed number to 16 bit signed number */
	if (battery_current >= 0x2000)
		battery_current = (battery_current - 0x4000);

	battery_current = battery_current * 3000 / 4096;
	printf("Battery Current: %d mA\n", battery_current);

	return battery_current;
}

int twl6030_get_battery_voltage(void)
{
	int battery_volt = 0;
	int ret = 0;

	/* Start GPADC SW conversion */
	ret = twl6030_gpadc_sw2_trigger();
	if (ret) {
		printf("Failed to convert battery voltage\n");
		return ret;
	}

	/* measure Vbat voltage */
	battery_volt = twl6030_gpadc_read_channel(7);
	if (battery_volt < 0) {
		printf("Failed to read battery voltage\n");
		return ret;
	}
	battery_volt = (battery_volt * 25 * 1000) >> (10 + 2);
	printf("Battery Voltage: %d mV\n", battery_volt);

	return battery_volt;
}

void twl6030_init_battery_charging(void)
{
	u8 stat1 = 0;
	int battery_volt = 0;
	int ret = 0;

	/* Enable VBAT measurement */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, MISC1, VBAT_MEAS);

	/* Enable GPADC module */
	ret = twl6030_i2c_write_u8(TWL6030_CHIP_CHARGER, TOGGLE1, FGS | GPADCS);
	if (ret) {
		printf("Failed to enable GPADC\n");
		return;
	}

	battery_volt = twl6030_get_battery_voltage();
	if (battery_volt < 0)
		return;

	if (battery_volt < 3000)
		printf("Main battery voltage too low!\n");

	/* Check for the presence of USB charger */
	twl6030_i2c_read_u8(TWL6030_CHIP_CHARGER, CONTROLLER_STAT1, &stat1);

	/* check for battery presence indirectly via Fuel gauge */
	if ((stat1 & VBUS_DET) && (battery_volt < 3300))
		twl6030_start_usb_charging();

	return;
}

void twl6030_power_mmc_init()
{
	/* set voltage to 3.0 and turnon for APP */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, VMMC_CFG_VOLTATE, 0x15);
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, VMMC_CFG_STATE, 0x21);
}

void twl6030_usb_device_settings()
{
	u8 data = 0;

	/* Select APP Group and set state to ON */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, VUSB_CFG_STATE, 0x21);

	twl6030_i2c_read_u8(TWL6030_CHIP_PM, MISC2, &data);
	data |= 0x10;

	/* Select the input supply for VBUS regulator */
	twl6030_i2c_write_u8(TWL6030_CHIP_PM, MISC2, data);
}
#endif

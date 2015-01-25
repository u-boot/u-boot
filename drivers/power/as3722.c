/*
 * Copyright (C) 2014 NVIDIA Corporation
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#define pr_fmt(fmt) "as3722: " fmt

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <i2c.h>

#include <power/as3722.h>

#define AS3722_SD_VOLTAGE(n) (0x00 + (n))
#define AS3722_GPIO_CONTROL(n) (0x08 + (n))
#define  AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDH (1 << 0)
#define  AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDL (7 << 0)
#define  AS3722_GPIO_CONTROL_INVERT (1 << 7)
#define AS3722_LDO_VOLTAGE(n) (0x10 + (n))
#define AS3722_GPIO_SIGNAL_OUT 0x20
#define AS3722_SD_CONTROL 0x4d
#define AS3722_LDO_CONTROL 0x4e
#define AS3722_ASIC_ID1 0x90
#define  AS3722_DEVICE_ID 0x0c
#define AS3722_ASIC_ID2 0x91

static int as3722_read(struct udevice *pmic, u8 reg, u8 *value)
{
	int err;

	err = dm_i2c_read(pmic, reg, value, 1);
	if (err < 0)
		return err;

	return 0;
}

static int as3722_write(struct udevice *pmic, u8 reg, u8 value)
{
	int err;

	err = dm_i2c_write(pmic, reg, &value, 1);
	if (err < 0)
		return err;

	return 0;
}

static int as3722_read_id(struct udevice *pmic, u8 *id, u8 *revision)
{
	int err;

	err = as3722_read(pmic, AS3722_ASIC_ID1, id);
	if (err) {
		error("failed to read ID1 register: %d", err);
		return err;
	}

	err = as3722_read(pmic, AS3722_ASIC_ID2, revision);
	if (err) {
		error("failed to read ID2 register: %d", err);
		return err;
	}

	return 0;
}

int as3722_sd_enable(struct udevice *pmic, unsigned int sd)
{
	u8 value;
	int err;

	if (sd > 6)
		return -EINVAL;

	err = as3722_read(pmic, AS3722_SD_CONTROL, &value);
	if (err) {
		error("failed to read SD control register: %d", err);
		return err;
	}

	value |= 1 << sd;

	err = as3722_write(pmic, AS3722_SD_CONTROL, value);
	if (err < 0) {
		error("failed to write SD control register: %d", err);
		return err;
	}

	return 0;
}

int as3722_sd_set_voltage(struct udevice *pmic, unsigned int sd, u8 value)
{
	int err;

	if (sd > 6)
		return -EINVAL;

	err = as3722_write(pmic, AS3722_SD_VOLTAGE(sd), value);
	if (err < 0) {
		error("failed to write SD%u voltage register: %d", sd, err);
		return err;
	}

	return 0;
}

int as3722_ldo_enable(struct udevice *pmic, unsigned int ldo)
{
	u8 value;
	int err;

	if (ldo > 11)
		return -EINVAL;

	err = as3722_read(pmic, AS3722_LDO_CONTROL, &value);
	if (err) {
		error("failed to read LDO control register: %d", err);
		return err;
	}

	value |= 1 << ldo;

	err = as3722_write(pmic, AS3722_LDO_CONTROL, value);
	if (err < 0) {
		error("failed to write LDO control register: %d", err);
		return err;
	}

	return 0;
}

int as3722_ldo_set_voltage(struct udevice *pmic, unsigned int ldo, u8 value)
{
	int err;

	if (ldo > 11)
		return -EINVAL;

	err = as3722_write(pmic, AS3722_LDO_VOLTAGE(ldo), value);
	if (err < 0) {
		error("failed to write LDO%u voltage register: %d", ldo,
		      err);
		return err;
	}

	return 0;
}

int as3722_gpio_configure(struct udevice *pmic, unsigned int gpio,
			  unsigned long flags)
{
	u8 value = 0;
	int err;

	if (flags & AS3722_GPIO_OUTPUT_VDDH)
		value |= AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDH;

	if (flags & AS3722_GPIO_INVERT)
		value |= AS3722_GPIO_CONTROL_INVERT;

	err = as3722_write(pmic, AS3722_GPIO_CONTROL(gpio), value);
	if (err) {
		error("failed to configure GPIO#%u: %d", gpio, err);
		return err;
	}

	return 0;
}

static int as3722_gpio_set(struct udevice *pmic, unsigned int gpio,
			   unsigned int level)
{
	const char *l;
	u8 value;
	int err;

	if (gpio > 7)
		return -EINVAL;

	err = as3722_read(pmic, AS3722_GPIO_SIGNAL_OUT, &value);
	if (err < 0) {
		error("failed to read GPIO signal out register: %d", err);
		return err;
	}

	if (level == 0) {
		value &= ~(1 << gpio);
		l = "low";
	} else {
		value |= 1 << gpio;
		l = "high";
	}

	err = as3722_write(pmic, AS3722_GPIO_SIGNAL_OUT, value);
	if (err) {
		error("failed to set GPIO#%u %s: %d", gpio, l, err);
		return err;
	}

	return 0;
}

int as3722_gpio_direction_output(struct udevice *pmic, unsigned int gpio,
				 unsigned int level)
{
	u8 value;
	int err;

	if (gpio > 7)
		return -EINVAL;

	if (level == 0)
		value = AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDL;
	else
		value = AS3722_GPIO_CONTROL_MODE_OUTPUT_VDDH;

	err = as3722_write(pmic, AS3722_GPIO_CONTROL(gpio), value);
	if (err) {
		error("failed to configure GPIO#%u as output: %d", gpio, err);
		return err;
	}

	err = as3722_gpio_set(pmic, gpio, level);
	if (err < 0) {
		error("failed to set GPIO#%u high: %d", gpio, err);
		return err;
	}

	return 0;
}

int as3722_init(struct udevice **devp)
{
	struct udevice *pmic;
	u8 id, revision;
	const unsigned int bus = 0;
	const unsigned int address = 0x40;
	int err;

	err = i2c_get_chip_for_busnum(bus, address, 1, &pmic);
	if (err)
		return err;
	err = as3722_read_id(pmic, &id, &revision);
	if (err < 0) {
		error("failed to read ID: %d", err);
		return err;
	}

	if (id != AS3722_DEVICE_ID) {
		error("unknown device");
		return -ENOENT;
	}

	debug("AS3722 revision %#x found on I2C bus %u, address %#x\n",
	      revision, bus, address);
	*devp = pmic;

	return 0;
}

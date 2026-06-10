// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * JEDEC JC-42.4 / TSE2004av Temperature Sensor driver.
 *
 * Generic I2C temperature sensor of the Serial Presence Detect (SPD)
 * bus of DDR3 and DDR4 SO-DIMMs / UDIMMs / RDIMMs per the JEDEC
 * JC-42.4 standard. The TSE2004av variant adds an integrated SPD
 * EEPROM, but the thermal register interface is the same and is
 * what this driver exposes.
 *
 * Register layout (subset):
 *   0x05  Ambient temperature, 16-bit big-endian:
 *           bit 15    : T_CRIT alarm flag    (read-only)
 *           bit 14    : T_HIGH alarm flag    (read-only)
 *           bit 13    : T_LOW alarm flag     (read-only)
 *           bit 12    : sign (two's-complement within bits[12:0])
 *           bits[11:0]: magnitude * 16 (LSB = 0.0625 degC = 62.5 mC)
 *   0x06  Manufacturer ID  (16-bit BE, JEP-106 vendor code)
 *   0x07  Device ID + Revision (upper byte = ID, lower = revision)
 *   ...
 */

#include <dm.h>
#include <i2c.h>
#include <thermal.h>
#include <linux/bitops.h>

#define JC42_REG_TEMP		0x05

#define JC42_TEMP_SIGN		BIT(12)
#define JC42_TEMP_MAGNITUDE	GENMASK(11, 0)

static int jc42_get_temp(struct udevice *dev, int *temp)
{
	u8 buf[2];
	int ret;
	int mag;

	ret = dm_i2c_read(dev, JC42_REG_TEMP, buf, sizeof(buf));
	if (ret)
		return ret;

	mag = ((buf[0] << 8) | buf[1]) & (JC42_TEMP_SIGN | JC42_TEMP_MAGNITUDE);
	if (mag & JC42_TEMP_SIGN)
		mag -= (JC42_TEMP_SIGN << 1);

	/*
	 * mag is in units of 1/16 degC. Multiply first to keep one
	 * extra bit of precision before the divide. Worst-case range
	 * for a 13-bit signed value is +/-4096, so the product fits
	 * comfortably in an int (~4.1M mC).
	 */
	*temp = mag * 1000 / 16;

	return 0;
}

static const struct dm_thermal_ops jc42_ops = {
	.get_temp	= jc42_get_temp,
};

/*
 * Optional DT label property override: it replace the default DM
 * device name (the ofnode name, eg "temp@18") so
 * temperature list or temperature get commands
 * show a human-meaningful identifier such as "ddr-top" or
 * "ddr-bottom".
 * It mirrors the Linux hwmon binding which uses label for the
 * per-sensor display name.
 */
static int jc42_bind(struct udevice *dev)
{
	const char *label = dev_read_string(dev, "label");

	if (label && *label)
		return device_set_name(dev, label);
	return 0;
}

static const struct udevice_id jc42_match[] = {
	{ .compatible = "jedec,jc-42.4-temp" },
	{ }
};

U_BOOT_DRIVER(jc42_thermal) = {
	.name		= "jc42_thermal",
	.id		= UCLASS_THERMAL,
	.of_match	= jc42_match,
	.bind		= jc42_bind,
	.ops		= &jc42_ops,
};

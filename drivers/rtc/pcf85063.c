// SPDX-License-Identifier: GPL-2.0-only
/*
 * PCF85063 and compatible I2C RTC driver
 *
 * Copyright (c) 2025 Kontron Europe GmbH.
 */

#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <dm/device_compat.h>

#define PCF85063_REG_CTRL1		0x00 /* status */
#define PCF85063_REG_CTRL1_SR		0x58

#define PCF85063_REG_SC			0x04 /* datetime */
#define PCF85063_REG_SC_OS		0x80

static int pcf85063_get_time(struct udevice *dev, struct rtc_time *tm)
{
	u8 regs[7];
	int ret;

	ret = dm_i2c_read(dev, PCF85063_REG_SC, regs, sizeof(regs));
	if (ret)
		return ret;

	if (regs[0] & PCF85063_REG_SC_OS) {
		dev_err(dev, "Power loss detected, Invalid time\n");
		return -EINVAL;
	}

	tm->tm_sec = bcd2bin(regs[0] & 0x7f);
	tm->tm_min = bcd2bin(regs[1] & 0x7f);
	tm->tm_hour = bcd2bin(regs[2] & 0x3f);
	tm->tm_mday = bcd2bin(regs[3] & 0x3f);
	tm->tm_wday = regs[4] & 0x07;
	tm->tm_mon = bcd2bin(regs[5] & 0x1f) - 1;
	tm->tm_year = bcd2bin(regs[6]) + 2000;

	return 0;
}

static int pcf85063_set_time(struct udevice *dev, const struct rtc_time *tm)
{
	u8 regs[7];

	if (tm->tm_year < 2000 || tm->tm_year > 2099) {
		dev_err(dev, "Year must be between 2000 and 2099.\n");
		return -EINVAL;
	}

	regs[0] = bin2bcd(tm->tm_sec);
	regs[1] = bin2bcd(tm->tm_min);
	regs[2] = bin2bcd(tm->tm_hour);
	regs[3] = bin2bcd(tm->tm_mday);
	regs[4] = tm->tm_wday;
	regs[5] = bin2bcd(tm->tm_mon + 1);
	regs[6] = bin2bcd(tm->tm_year % 100);

	return dm_i2c_write(dev, PCF85063_REG_SC, regs, sizeof(regs));
}

static int pcf85063_reset(struct udevice *dev)
{
	return dm_i2c_reg_write(dev, PCF85063_REG_CTRL1, PCF85063_REG_CTRL1_SR);
}

static int pcf85063_read(struct udevice *dev, unsigned int offset, u8 *buf,
			 unsigned int len)
{
	return dm_i2c_read(dev, offset, buf, len);
}

static int pcf85063_write(struct udevice *dev, unsigned int offset,
			  const u8 *buf, unsigned int len)
{
	return dm_i2c_write(dev, offset, buf, len);
}

static const struct rtc_ops pcf85063_rtc_ops = {
	.get = pcf85063_get_time,
	.set = pcf85063_set_time,
	.reset = pcf85063_reset,
	.read = pcf85063_read,
	.write = pcf85063_write,
};

static int pcf85063_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct udevice_id pcf85063_of_id[] = {
	{ .compatible = "microcrystal,rv8263" },
	{ }
};

U_BOOT_DRIVER(rtc_pcf85063) = {
	.name	= "rtc-pcf85063",
	.id     = UCLASS_RTC,
	.probe  = pcf85063_probe,
	.of_match = pcf85063_of_id,
	.ops    = &pcf85063_rtc_ops,
};

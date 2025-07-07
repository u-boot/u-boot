// SPDX-License-Identifier: GPL-2.0-only
/*
 * Analog Devices DS1672 I2C RTC driver
 *
 * Copyright 2025 Gateworks Corporation.
 */
#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <dm/device_compat.h>

/* Registers */
#define DS1672_REG_CNT_BASE	0
#define DS1672_REG_CONTROL	4
#define DS1672_REG_TRICKLE	5

#define DS1672_REG_CONTROL_EOSC	0x80

static int ds1672_read_time(struct udevice *dev, struct rtc_time *tm)
{
	time64_t secs;
	u8 regs[4];
	int ret;

	ret = dm_i2c_read(dev, DS1672_REG_CONTROL, regs, 1);
	if (ret)
		return ret;

	if (regs[0] & DS1672_REG_CONTROL_EOSC) {
		dev_err(dev, "Oscillator not enabled. Set time to enable.\n");
		return -EINVAL;
	}

	ret = dm_i2c_read(dev, DS1672_REG_CNT_BASE, regs, 4);
	if (ret)
		return ret;
	dev_dbg(dev, "raw read: 0x%02x 0x%02x 0x%02x 0x%02x\n",
		regs[0], regs[1], regs[2], regs[3]);
	secs = ((unsigned long)regs[3] << 24) | (regs[2] << 16) | (regs[1] << 8) | regs[0];
	rtc_to_tm(secs, tm);

	dev_dbg(dev, "read %lld %4d-%02d-%02d (wday=%d) %2d:%02d:%02d\n", secs,
		tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int ds1672_set_time(struct udevice *dev, const struct rtc_time *tm)
{
	time64_t secs = rtc_mktime(tm);
	u8 regs[5];

	dev_dbg(dev, "set %4d-%02d-%02d (wday=%d) %2d:%02d:%02d %lld\n",
		tm->tm_year, tm->tm_mon, tm->tm_mday,
		tm->tm_wday, tm->tm_hour, tm->tm_min, tm->tm_sec,
		secs);

	if (tm->tm_year < 2000) {
		dev_err(dev, "year %d (before 2000) not supported\n",
			tm->tm_year);
		return -EINVAL;
	}

	regs[0] = secs & 0x000000ff;
	regs[1] = (secs & 0x0000ff00) >> 8;
	regs[2] = (secs & 0x00ff0000) >> 16;
	regs[3] = (secs & 0xff000000) >> 24;
	regs[4] = 0; /* set control reg to enable counting */

	return dm_i2c_write(dev, DS1672_REG_CNT_BASE, regs, 5);
}

static int ds1672_reset(struct udevice *dev)
{
	u8 regs[5] = { 0 };

	return dm_i2c_write(dev, DS1672_REG_CNT_BASE, regs, 5);
}

static int ds1672_read8(struct udevice *dev, unsigned int reg)
{
	return  dm_i2c_reg_read(dev, reg);
}

static int ds1672_write8(struct udevice *dev, unsigned int reg, int val)
{
	return dm_i2c_reg_write(dev, reg, val);
}

static const struct rtc_ops ds1672_rtc_ops = {
	.get	= ds1672_read_time,
	.set	= ds1672_set_time,
	.reset  = ds1672_reset,
	.read8	= ds1672_read8,
	.write8	= ds1672_write8,
};

static int ds1672_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct udevice_id ds1672_of_id[] = {
	{ .compatible = "dallas,ds1672" },
	{ }
};

U_BOOT_DRIVER(rtc_max313xx) = {
	.name	= "rtc-ds1672",
	.id     = UCLASS_RTC,
	.probe  = ds1672_probe,
	.of_match = ds1672_of_id,
	.ops    = &ds1672_rtc_ops,
};

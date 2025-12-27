// SPDX-License-Identifier: GPL-2.0-only
/*
 * Micro Crystal RV3032 I2C RTC driver
 *
 * Copyright (c) 2025 Kontron Europe GmbH.
 */

#include <dm.h>
#include <i2c.h>
#include <rtc.h>
#include <dm/device_compat.h>

#define RV3032_REG_SEC			0x01
#define RV3032_REG_STATUS		0x0d
#define RV3032_REG_STATUS_VLF		BIT(0)
#define RV3032_REG_STATUS_PORF		BIT(1)

static int rv3032_get_time(struct udevice *dev, struct rtc_time *tm)
{
	int ret, status;
	u8 regs[7];

	status = dm_i2c_reg_read(dev, RV3032_REG_STATUS);
	if (status < 0)
		return status;

	if (status & (RV3032_REG_STATUS_PORF | RV3032_REG_STATUS_VLF)) {
		dev_err(dev, "Power loss detected, Invalid time\n");
		return -EINVAL;
	}

	ret = dm_i2c_read(dev, RV3032_REG_SEC, regs, sizeof(regs));
	if (ret)
		return ret;

	tm->tm_sec = bcd2bin(regs[0] & 0x7f);
	tm->tm_min = bcd2bin(regs[1] & 0x7f);
	tm->tm_hour = bcd2bin(regs[2] & 0x3f);
	tm->tm_mday = bcd2bin(regs[3] & 0x3f);
	tm->tm_wday = regs[4] & 0x07;
	tm->tm_mon = bcd2bin(regs[5] & 0x1f) - 1;
	tm->tm_year = bcd2bin(regs[6]) + 2000;

	return 0;
}

static int rv3032_set_time(struct udevice *dev, const struct rtc_time *tm)
{
	u8 regs[7];
	int ret;

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

	ret = dm_i2c_write(dev, RV3032_REG_SEC, regs, sizeof(regs));
	if (ret)
		return ret;

	return dm_i2c_reg_clrset(dev, RV3032_REG_STATUS,
				 RV3032_REG_STATUS_PORF | RV3032_REG_STATUS_VLF,
				 0);
}

static int rv3032_read(struct udevice *dev, unsigned int offset, u8 *buf,
		       unsigned int len)
{
	return dm_i2c_read(dev, offset, buf, len);
}

static int rv3032_write(struct udevice *dev, unsigned int offset,
			const u8 *buf, unsigned int len)
{
	return dm_i2c_write(dev, offset, buf, len);
}

static int rv3032_reset(struct udevice *dev)
{
	/*
	 * There is no reset, but the "date reset" command needs this op to
	 * actually set the default time
	 */
	return 0;
}

static const struct rtc_ops rv3032_rtc_ops = {
	.get = rv3032_get_time,
	.set = rv3032_set_time,
	.read = rv3032_read,
	.write = rv3032_write,
	.reset = rv3032_reset,
};

static int rv3032_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct udevice_id rv3032_of_id[] = {
	{ .compatible = "microcrystal,rv3032" },
	{ }
};

U_BOOT_DRIVER(rtc_rv3032) = {
	.name	= "rtc-rv3032",
	.id     = UCLASS_RTC,
	.probe  = rv3032_probe,
	.of_match = rv3032_of_id,
	.ops    = &rv3032_rtc_ops,
};

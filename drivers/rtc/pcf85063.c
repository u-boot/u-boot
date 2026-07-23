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
#define PCF85063_REG_CTRL1_CAP_SEL	BIT(0)
#define PCF85063_REG_CTRL1_STOP		BIT(5)
#define PCF85063_REG_CTRL1_EXT_TEST	BIT(7)
#define PCF85063_REG_CTRL1_SWR		0x58 /* Software reset command */

#define PCF85063_REG_CTRL2		0x01
#define PCF85063_CTRL2_AF		BIT(6)
#define PCF85063_CTRL2_AIE		BIT(7)

#define PCF85063_REG_OFFSET		0x02
#define PCF85063_OFFSET_SIGN_BIT	6	/* 2's complement sign bit */
#define PCF85063_OFFSET_MODE		BIT(7)
#define PCF85063_OFFSET_STEP0		4340
#define PCF85063_OFFSET_STEP1		4069

#define PCF85063_REG_CLKO_F_MASK	0x07 /* frequency mask */
#define PCF85063_REG_CLKO_F_32768HZ	0x00
#define PCF85063_REG_CLKO_F_OFF		0x07

#define PCF85063_REG_RAM		0x03

#define PCF85063_REG_SC			0x04 /* datetime */
#define PCF85063_REG_SC_OS		0x80

#define PCF85063_REG_ALM_S		0x0b
#define PCF85063_AEN			BIT(7)

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
	/* rtc register and rtc_time spec uses 1 - 12 */
	tm->tm_mon = bcd2bin(regs[5] & 0x1f);
	/* adjust rtc_time (years since 0) to match register spec */
	tm->tm_year = bcd2bin(regs[6]) + 2000;

	return 0;
}

static int pcf85063_set_time(struct udevice *dev, const struct rtc_time *tm)
{
	u8 regs[7];
	int rc;

	if (tm->tm_year < 2000 || tm->tm_year > 2099) {
		dev_err(dev, "Year must be between 2000 and 2099.\n");
		return -EINVAL;
	}

	/*
	 * to accurately set the time, reset the divider chain and keep it in
	 * reset state until all time/date registers are written
	 */
	rc = dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
			       PCF85063_REG_CTRL1_EXT_TEST |
			       PCF85063_REG_CTRL1_STOP,
			       PCF85063_REG_CTRL1_STOP);

	if (rc)
		return rc;

	/* hours, minutes and seconds */
	regs[0] = bin2bcd(tm->tm_sec) & (~PCF85063_REG_SC_OS);

	regs[1] = bin2bcd(tm->tm_min);
	regs[2] = bin2bcd(tm->tm_hour);

	/* Day of month, 1 - 31 */
	regs[3] = bin2bcd(tm->tm_mday);

	/* Day of week 0 - 6 */
	regs[4] = tm->tm_wday & 0x07;

	/* rtc register and rtc_time spec uses 1 - 12 */
	regs[5] = bin2bcd(tm->tm_mon);
	/* adjust register to match rtc_time spec */
	regs[6] = bin2bcd(tm->tm_year % 100);

	rc = dm_i2c_write(dev, PCF85063_REG_SC, regs, sizeof(regs));
	if (rc)
		return rc;

	/*
	 * Write the control register as a separate action since the size of
	 * the register space is different between the PCF85063TP and
	 * PCF85063A devices. The rollover point can not be used.
	 */
	return dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
				 PCF85063_REG_CTRL1_STOP, 0);
}

static int pcf85063_reset(struct udevice *dev)
{
	return dm_i2c_reg_write(dev, PCF85063_REG_CTRL1, PCF85063_REG_CTRL1_SWR);
}

static int pcf85063_read(struct udevice *dev, unsigned int offset, u8 *buf,
			 unsigned int len)
{
	if (offset + len > dev->driver_data)
		return -EINVAL;

	return dm_i2c_read(dev, offset, buf, len);
}

static int pcf85063_write(struct udevice *dev, unsigned int offset,
			  const u8 *buf, unsigned int len)
{
	if (offset + len > dev->driver_data)
		return -EINVAL;

	return dm_i2c_write(dev, offset, buf, len);
}

static int pcf85063_load_capacitance(struct udevice *dev)
{
	u32 load = 7000;
	u8 reg = 0;

	if (ofnode_read_u32(dev_ofnode(dev), "quartz-load-femtofarads", &load))
		return 0;

	switch (load) {
	default:
		dev_warn(dev, "Unknown quartz-load-femtofarads value: %d. Assuming 7000",
			 load);
		fallthrough;
	case 7000:
		break;
	case 12500:
		reg = PCF85063_REG_CTRL1_CAP_SEL;
		break;
	}

	return dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
				 PCF85063_REG_CTRL1_CAP_SEL, reg);
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
	u8 tmp;
	int err;

	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS | DM_I2C_CHIP_WR_ADDRESS);

	err = dm_i2c_read(dev, PCF85063_REG_SC, &tmp, sizeof(tmp));
	if (err) {
		dev_err(dev, "RTC chip is not present\n");
		return err;
	}

	/*
	 * If a Power loss is detected, SW reset the device.
	 * From PCF85063A datasheet:
	 * There is a low probability that some devices will have corruption
	 * of the registers after the automatic power-on reset...
	 */
	if (tmp & PCF85063_REG_SC_OS) {
		dev_warn(dev, "POR issue detected, sending a SW reset\n");
		err = dm_i2c_reg_clrset(dev, PCF85063_REG_CTRL1,
					0xff, PCF85063_REG_CTRL1_SWR);
		if (err < 0)
			dev_warn(dev, "SW reset failed, trying to continue\n");
	}

	err = pcf85063_load_capacitance(dev);
	if (err < 0)
		dev_warn(dev, "failed to set xtal load capacitance: %d",
			 err);

	return 0;
}

static const struct udevice_id pcf85063_of_id[] = {
	{ .compatible = "microcrystal,rv8263", .data = 0x12 },
	{ .compatible = "nxp,pcf85063", .data = 0xb },
	{ .compatible = "nxp,pcf85063a", .data = 0x12 },
	{ .compatible = "nxp,pcf85063tp", .data = 0xb },
	{ .compatible = "nxp,pcf85073a", .data = 0x12 },
	{ }
};

U_BOOT_DRIVER(rtc_pcf85063) = {
	.name	= "rtc-pcf85063",
	.id     = UCLASS_RTC,
	.probe  = pcf85063_probe,
	.of_match = pcf85063_of_id,
	.ops    = &pcf85063_rtc_ops,
};

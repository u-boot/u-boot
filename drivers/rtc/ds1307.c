// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001, 2002, 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com`
 * Steven Scholz, steven.scholz@imc-berlin.de
 */

/*
 * Date & Time support (no alarms) for Dallas Semiconductor (now Maxim)
 * DS1307 and DS1338/9 Real Time Clock (RTC).
 *
 * based on ds1337.c
 */

#include <config.h>
#include <command.h>
#include <dm.h>
#include <log.h>
#include <rtc.h>
#include <i2c.h>

enum ds_type {
	ds_1307,
	ds_1337,
	ds_1339,
	ds_1340,
	m41t11,
	mcp794xx,
};

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x00
#define RTC_MIN_REG_ADDR	0x01
#define RTC_HR_REG_ADDR		0x02
#define RTC_DAY_REG_ADDR	0x03
#define RTC_DATE_REG_ADDR	0x04
#define RTC_MON_REG_ADDR	0x05
#define RTC_YR_REG_ADDR		0x06
#define RTC_CTL_REG_ADDR	0x07

#define DS1337_CTL_REG_ADDR	0x0e
#define DS1337_STAT_REG_ADDR	0x0f
#define DS1340_STAT_REG_ADDR	0x09

#define RTC_STAT_BIT_OSF	0x80

#define RTC_SEC_BIT_CH		0x80	/* Clock Halt (in Register 0)   */

/* DS1307-specific bits */
#define RTC_CTL_BIT_RS0		0x01	/* Rate select 0                */
#define RTC_CTL_BIT_RS1		0x02	/* Rate select 1                */
#define RTC_CTL_BIT_SQWE	0x10	/* Square Wave Enable           */
#define RTC_CTL_BIT_OUT		0x80	/* Output Control               */

/* DS1337-specific bits */
#define DS1337_CTL_BIT_RS1	0x08	/* Rate select 1                */
#define DS1337_CTL_BIT_RS2	0x10	/* Rate select 2                */
#define DS1337_CTL_BIT_EOSC	0x80	/* Enable Oscillator            */

/* DS1340-specific bits */
#define DS1340_SEC_BIT_EOSC	0x80	/* Enable Oscillator            */
#define DS1340_CTL_BIT_OUT	0x80	/* Output Control               */

/* MCP7941X-specific bits */
#define MCP7941X_BIT_ST		0x80
#define MCP7941X_BIT_VBATEN	0x08

static int ds1307_rtc_set(struct udevice *dev, const struct rtc_time *tm)
{
	int ret;
	uchar buf[7];
	enum ds_type type = dev_get_driver_data(dev);

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (tm->tm_year < 1970 || tm->tm_year > 2069)
		printf("WARNING: year should be between 1970 and 2069!\n");

	buf[RTC_YR_REG_ADDR] = bin2bcd(tm->tm_year % 100);
	buf[RTC_MON_REG_ADDR] = bin2bcd(tm->tm_mon);
	buf[RTC_DAY_REG_ADDR] = bin2bcd(tm->tm_wday + 1);
	buf[RTC_DATE_REG_ADDR] = bin2bcd(tm->tm_mday);
	buf[RTC_HR_REG_ADDR] = bin2bcd(tm->tm_hour);
	buf[RTC_MIN_REG_ADDR] = bin2bcd(tm->tm_min);
	buf[RTC_SEC_REG_ADDR] = bin2bcd(tm->tm_sec);

	if (type == mcp794xx) {
		buf[RTC_DAY_REG_ADDR] |= MCP7941X_BIT_VBATEN;
		buf[RTC_SEC_REG_ADDR] |= MCP7941X_BIT_ST;
	}

	ret = dm_i2c_write(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (type == ds_1337) {
		/* Ensure oscillator is enabled */
		dm_i2c_reg_write(dev, DS1337_CTL_REG_ADDR, 0);
	}

	return 0;
}

static int ds1307_rtc_get(struct udevice *dev, struct rtc_time *tm)
{
	int ret;
	uchar buf[7];
	enum ds_type type = dev_get_driver_data(dev);

	ret = dm_i2c_read(dev, 0, buf, sizeof(buf));
	if (ret < 0)
		return ret;

	if (type == ds_1337 || type == ds_1340) {
		uint reg = (type == ds_1337) ? DS1337_STAT_REG_ADDR :
					       DS1340_STAT_REG_ADDR;
		int status = dm_i2c_reg_read(dev, reg);

		if (status >= 0 && (status & RTC_STAT_BIT_OSF)) {
			printf("### Warning: RTC oscillator has stopped\n");
			/* clear the OSF flag */
			dm_i2c_reg_write(dev, reg, status & ~RTC_STAT_BIT_OSF);
		}
	}

	tm->tm_sec  = bcd2bin(buf[RTC_SEC_REG_ADDR] & 0x7F);
	tm->tm_min  = bcd2bin(buf[RTC_MIN_REG_ADDR] & 0x7F);
	tm->tm_hour = bcd2bin(buf[RTC_HR_REG_ADDR] & 0x3F);
	tm->tm_mday = bcd2bin(buf[RTC_DATE_REG_ADDR] & 0x3F);
	tm->tm_mon  = bcd2bin(buf[RTC_MON_REG_ADDR] & 0x1F);
	tm->tm_year = bcd2bin(buf[RTC_YR_REG_ADDR]) +
			      (bcd2bin(buf[RTC_YR_REG_ADDR]) >= 70 ?
			       1900 : 2000);
	tm->tm_wday = bcd2bin((buf[RTC_DAY_REG_ADDR] - 1) & 0x07);
	tm->tm_yday = 0;
	tm->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_wday,
	      tm->tm_hour, tm->tm_min, tm->tm_sec);

	return 0;
}

static int ds1307_rtc_reset(struct udevice *dev)
{
	int ret;
	enum ds_type type = dev_get_driver_data(dev);

	/*
	 * reset clock/oscillator in the seconds register:
	 * on DS1307 bit 7 enables Clock Halt (CH),
	 * on DS1340 bit 7 disables the oscillator (not EOSC)
	 * on MCP794xx bit 7 enables Start Oscillator (ST)
	 */
	ret = dm_i2c_reg_write(dev, RTC_SEC_REG_ADDR, 0x00);
	if (ret < 0)
		return ret;

	if (type == ds_1307) {
		/* Write control register in order to enable square-wave
		 * output (SQWE) and set a default rate of 32.768kHz (RS1|RS0).
		 */
		ret = dm_i2c_reg_write(dev, RTC_CTL_REG_ADDR,
				       RTC_CTL_BIT_SQWE | RTC_CTL_BIT_RS1 |
				       RTC_CTL_BIT_RS0);
	} else if (type == ds_1337) {
		/* Write control register in order to enable oscillator output
		 * (not EOSC) and set a default rate of 32.768kHz (RS2|RS1).
		 */
		ret = dm_i2c_reg_write(dev, DS1337_CTL_REG_ADDR,
				       DS1337_CTL_BIT_RS2 | DS1337_CTL_BIT_RS1);
	} else if (type == ds_1340 || type == mcp794xx || type == m41t11) {
		/* Reset clock calibration, frequency test and output level. */
		ret = dm_i2c_reg_write(dev, RTC_CTL_REG_ADDR, 0x00);
	}

	return ret;
}

static int ds1307_probe(struct udevice *dev)
{
	i2c_set_chip_flags(dev, DM_I2C_CHIP_RD_ADDRESS |
			   DM_I2C_CHIP_WR_ADDRESS);

	return 0;
}

static const struct rtc_ops ds1307_rtc_ops = {
	.get = ds1307_rtc_get,
	.set = ds1307_rtc_set,
	.reset = ds1307_rtc_reset,
};

static const struct udevice_id ds1307_rtc_ids[] = {
	{ .compatible = "dallas,ds1307", .data = ds_1307 },
	{ .compatible = "dallas,ds1337", .data = ds_1337 },
	{ .compatible = "dallas,ds1339", .data = ds_1339 },
	{ .compatible = "dallas,ds1340", .data = ds_1340 },
	{ .compatible = "microchip,mcp7940x", .data = mcp794xx },
	{ .compatible = "microchip,mcp7941x", .data = mcp794xx },
	{ .compatible = "st,m41t11", .data = m41t11 },
	{ }
};

U_BOOT_DRIVER(rtc_ds1307) = {
	.name	= "rtc-ds1307",
	.id	= UCLASS_RTC,
	.probe	= ds1307_probe,
	.of_match = ds1307_rtc_ids,
	.ops	= &ds1307_rtc_ops,
};

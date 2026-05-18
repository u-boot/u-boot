// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001-2008
 * Copyright 2020 NXP
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * Keith Outwater, keith_outwater@mvis.com`
 */

/*
 * Date & Time support (no alarms) for Dallas Semiconductor (now Maxim)
 * DS1337 Real Time Clock (RTC).
 */

#include <config.h>
#include <command.h>
#include <dm.h>
#include <log.h>
#include <rtc.h>
#include <i2c.h>

/*
 * RTC register addresses
 */
#define RTC_SEC_REG_ADDR	0x0
#define RTC_MIN_REG_ADDR	0x1
#define RTC_HR_REG_ADDR		0x2
#define RTC_DAY_REG_ADDR	0x3
#define RTC_DATE_REG_ADDR	0x4
#define RTC_MON_REG_ADDR	0x5
#define RTC_YR_REG_ADDR		0x6
#define RTC_CTL_REG_ADDR	0x0e
#define RTC_STAT_REG_ADDR	0x0f
#define RTC_TC_REG_ADDR		0x10

/*
 * RTC control register bits
 */
#define RTC_CTL_BIT_A1IE	0x1	/* Alarm 1 interrupt enable	*/
#define RTC_CTL_BIT_A2IE	0x2	/* Alarm 2 interrupt enable	*/
#define RTC_CTL_BIT_INTCN	0x4	/* Interrupt control		*/
#define RTC_CTL_BIT_RS1		0x8	/* Rate select 1		*/
#define RTC_CTL_BIT_RS2		0x10	/* Rate select 2		*/
#define RTC_CTL_BIT_DOSC	0x80	/* Disable Oscillator		*/

/*
 * RTC status register bits
 */
#define RTC_STAT_BIT_A1F	0x1	/* Alarm 1 flag			*/
#define RTC_STAT_BIT_A2F	0x2	/* Alarm 2 flag			*/
#define RTC_STAT_BIT_OSF	0x80	/* Oscillator stop flag		*/

static uchar rtc_read(struct udevice *dev, uchar reg)
{
	return dm_i2c_reg_read(dev, reg);
}

static void rtc_write(struct udevice *dev, uchar reg, uchar val)
{
	dm_i2c_reg_write(dev, reg, val);
}

static int ds1337_rtc_get(struct udevice *dev, struct rtc_time *tmp)
{
	int rel = 0;
	uchar sec, min, hour, mday, wday, mon_cent, year, control, status;

	control = rtc_read(dev, RTC_CTL_REG_ADDR);
	status = rtc_read(dev, RTC_STAT_REG_ADDR);
	sec = rtc_read(dev, RTC_SEC_REG_ADDR);
	min = rtc_read(dev, RTC_MIN_REG_ADDR);
	hour = rtc_read(dev, RTC_HR_REG_ADDR);
	wday = rtc_read(dev, RTC_DAY_REG_ADDR);
	mday = rtc_read(dev, RTC_DATE_REG_ADDR);
	mon_cent = rtc_read(dev, RTC_MON_REG_ADDR);
	year = rtc_read(dev, RTC_YR_REG_ADDR);

	debug("Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x\n",
	      year, mon_cent, mday, wday);
	debug("hr: %02x min: %02x sec: %02x control: %02x status: %02x\n",
	      hour, min, sec, control, status);

	if (status & RTC_STAT_BIT_OSF) {
		printf("### Warning: RTC oscillator has stopped\n");
		/* clear the OSF flag */
		rtc_write(dev, RTC_STAT_REG_ADDR,
			  rtc_read(dev, RTC_STAT_REG_ADDR) & ~RTC_STAT_BIT_OSF);
		rel = -1;
	}

	tmp->tm_sec = bcd2bin(sec & 0x7F);
	tmp->tm_min = bcd2bin(min & 0x7F);
	tmp->tm_hour = bcd2bin(hour & 0x3F);
	tmp->tm_mday = bcd2bin(mday & 0x3F);
	tmp->tm_mon  = bcd2bin(mon_cent & 0x1F);
	tmp->tm_year = bcd2bin(year) + ((mon_cent & 0x80) ? 2000 : 1900);
	tmp->tm_wday = bcd2bin((wday - 1) & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return rel;
}

static int ds1337_rtc_set(struct udevice *dev, const struct rtc_time *tmp)
{
	uchar century;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write(dev, RTC_YR_REG_ADDR, bin2bcd(tmp->tm_year % 100));

	century = (tmp->tm_year >= 2000) ? 0x80 : 0;
	rtc_write(dev, RTC_MON_REG_ADDR, bin2bcd(tmp->tm_mon) | century);

	rtc_write(dev, RTC_DAY_REG_ADDR, bin2bcd(tmp->tm_wday + 1));
	rtc_write(dev, RTC_DATE_REG_ADDR, bin2bcd(tmp->tm_mday));
	rtc_write(dev, RTC_HR_REG_ADDR, bin2bcd(tmp->tm_hour));
	rtc_write(dev, RTC_MIN_REG_ADDR, bin2bcd(tmp->tm_min));
	rtc_write(dev, RTC_SEC_REG_ADDR, bin2bcd(tmp->tm_sec));

	return 0;
}

#ifdef CONFIG_RTC_DS1337_NOOSC
 #define RTC_DS1337_RESET_VAL \
	(RTC_CTL_BIT_INTCN | RTC_CTL_BIT_RS1 | RTC_CTL_BIT_RS2)
#else
 #define RTC_DS1337_RESET_VAL (RTC_CTL_BIT_RS1 | RTC_CTL_BIT_RS2)
#endif
static int ds1337_rtc_reset(struct udevice *dev)
{
	rtc_write(dev, RTC_CTL_REG_ADDR, RTC_DS1337_RESET_VAL);

	return 0;
}

static const struct rtc_ops ds1337_rtc_ops = {
	.get = ds1337_rtc_get,
	.set = ds1337_rtc_set,
	.reset = ds1337_rtc_reset,
};

static const struct udevice_id ds1337_rtc_ids[] = {
	{ .compatible = "ds1337" },
	{ .compatible = "ds1338" },
	{ .compatible = "ds1339" },
	{ }
};

U_BOOT_DRIVER(rtc_ds1337) = {
	.name   = "rtc-ds1337",
	.id     = UCLASS_RTC,
	.of_match = ds1337_rtc_ids,
	.ops    = &ds1337_rtc_ops,
};

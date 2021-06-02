// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 DENX Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
 */
#include <common.h>
#include <command.h>
#include <log.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/davinci_rtc.h>
#include <asm/arch/hardware.h>
#include <linux/delay.h>

#if !defined(RTC_BASE) && defined(DAVINCI_RTC_BASE)
#define RTC_BASE DAVINCI_RTC_BASE
#endif

static void davinci_rtc_lock(struct davinci_rtc *rtc)
{
	writel(0, &rtc->kick0r);
	writel(0, &rtc->kick1r);
}

static void davinci_rtc_unlock(struct davinci_rtc *rtc)
{
	writel(RTC_KICK0R_WE, &rtc->kick0r);
	writel(RTC_KICK1R_WE, &rtc->kick1r);
}

static int davinci_rtc_wait_not_busy(struct davinci_rtc *rtc)
{
	int count;
	u8 status;

	status = readb(&rtc->status);
	if ((status & RTC_STATE_RUN) != RTC_STATE_RUN) {
		printf("RTC doesn't run\n");
		return -1;
	}

	/* BUSY may stay active for 1/32768 second (~30 usec) */
	for (count = 0; count < 50; count++) {
		if (!(status & RTC_STATE_BUSY))
			break;

		udelay(1);
		status = readb(&rtc->status);
	}

	/* now we have ~15 usec to read/write various registers */
	return 0;
}

int rtc_get(struct rtc_time *tmp)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;
	unsigned long sec, min, hour, mday, wday, mon_cent, year;
	int ret;

	ret = davinci_rtc_wait_not_busy(rtc);
	if (ret)
		return ret;

	sec	= readb(&rtc->second);
	min	= readb(&rtc->minutes);
	hour	= readb(&rtc->hours);
	mday	= readb(&rtc->day);
	wday	= readb(&rtc->dotw);
	mon_cent = readb(&rtc->month);
	year	= readb(&rtc->year);

	debug("Get RTC year: %02lx mon/cent: %02lx mday: %02lx wday: %02lx "
		"hr: %02lx min: %02lx sec: %02lx\n",
		year, mon_cent, mday, wday,
		hour, min, sec);

	tmp->tm_sec  = bcd2bin(sec  & 0x7F);
	tmp->tm_min  = bcd2bin(min  & 0x7F);
	tmp->tm_hour = bcd2bin(hour & 0x3F);
	tmp->tm_mday = bcd2bin(mday & 0x3F);
	tmp->tm_mon  = bcd2bin(mon_cent & 0x1F);
	tmp->tm_year = bcd2bin(year) + 2000;
	tmp->tm_wday = bcd2bin(wday & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

	debug("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

int rtc_set(struct rtc_time *tmp)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;
	int ret;

	ret = davinci_rtc_wait_not_busy(rtc);
	if (ret)
		return ret;

	davinci_rtc_unlock(rtc);
	writeb(bin2bcd(tmp->tm_year % 100), &rtc->year);
	writeb(bin2bcd(tmp->tm_mon), &rtc->month);

	writeb(bin2bcd(tmp->tm_wday), &rtc->dotw);
	writeb(bin2bcd(tmp->tm_mday), &rtc->day);
	writeb(bin2bcd(tmp->tm_hour), &rtc->hours);
	writeb(bin2bcd(tmp->tm_min), &rtc->minutes);
	writeb(bin2bcd(tmp->tm_sec), &rtc->second);
	davinci_rtc_lock(rtc);

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

void rtc_reset(void)
{
	struct davinci_rtc *rtc = (struct davinci_rtc *)RTC_BASE;

	/* run RTC counter */
	writeb(0x01, &rtc->ctrl);
}

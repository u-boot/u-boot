/*
 * (C) Copyright 2011 DENX Software Engineering GmbH
 * Heiko Schocher <hs@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#include <common.h>
#include <command.h>
#include <rtc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

#if defined(CONFIG_CMD_DATE)
struct davinci_rtc {
	u_int32_t	second;
	u_int32_t	minutes;
	u_int32_t	hours;
	u_int32_t	day;
	u_int32_t	month; /* 0x10 */
	u_int32_t	year;
	u_int32_t	dotw;
	u_int32_t	resv1;
	u_int32_t	alarmsecond; /* 0x20 */
	u_int32_t	alarmminute;
	u_int32_t	alarmhour;
	u_int32_t	alarmday;
	u_int32_t	alarmmonth; /* 0x30 */
	u_int32_t	alarmyear;
	u_int32_t	resv2[2];
	u_int32_t	ctrl; /* 0x40 */
	u_int32_t	status;
	u_int32_t	irq;
};

#define RTC_STATE_BUSY	0x01
#define RTC_STATE_RUN	0x02

#define davinci_rtc_base ((struct davinci_rtc *)DAVINCI_RTC_BASE)

int rtc_get(struct rtc_time *tmp)
{
	struct davinci_rtc *rtc = davinci_rtc_base;
	unsigned long sec, min, hour, mday, wday, mon_cent, year;
	unsigned long status;

	status = readl(&rtc->status);
	if ((status & RTC_STATE_RUN) != RTC_STATE_RUN) {
		printf("RTC doesn't run\n");
		return -1;
	}
	if ((status & RTC_STATE_BUSY) == RTC_STATE_BUSY)
		udelay(20);

	sec	= readl(&rtc->second);
	min	= readl(&rtc->minutes);
	hour	= readl(&rtc->hours);
	mday	= readl(&rtc->day);
	wday	= readl(&rtc->dotw);
	mon_cent = readl(&rtc->month);
	year	= readl(&rtc->year);

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
	struct davinci_rtc *rtc = davinci_rtc_base;

	debug("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
	writel(bin2bcd(tmp->tm_year % 100), &rtc->year);
	writel(bin2bcd(tmp->tm_mon), &rtc->month);

	writel(bin2bcd(tmp->tm_wday), &rtc->dotw);
	writel(bin2bcd(tmp->tm_mday), &rtc->day);
	writel(bin2bcd(tmp->tm_hour), &rtc->hours);
	writel(bin2bcd(tmp->tm_min), &rtc->minutes);
	writel(bin2bcd(tmp->tm_sec), &rtc->second);
	return 0;
}

void rtc_reset(void)
{
	struct davinci_rtc *rtc = davinci_rtc_base;

	/* run RTC counter */
	writel(0x01, &rtc->ctrl);
}
#endif

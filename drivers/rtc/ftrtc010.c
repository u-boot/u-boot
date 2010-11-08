/*
 * Faraday FTRTC010 Real Time Clock
 *
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <common.h>
#include <rtc.h>
#include <asm/io.h>

struct ftrtc010 {
	unsigned int sec;		/* 0x00 */
	unsigned int min;		/* 0x04 */
	unsigned int hour;		/* 0x08 */
	unsigned int day;		/* 0x0c */
	unsigned int alarm_sec;		/* 0x10 */
	unsigned int alarm_min;		/* 0x14 */
	unsigned int alarm_hour;	/* 0x18 */
	unsigned int record;		/* 0x1c */
	unsigned int cr;		/* 0x20 */
};

/*
 * RTC Control Register
 */
#define FTRTC010_CR_ENABLE		(1 << 0)
#define FTRTC010_CR_INTERRUPT_SEC	(1 << 1)	/* per second irq */
#define FTRTC010_CR_INTERRUPT_MIN	(1 << 2)	/* per minute irq */
#define FTRTC010_CR_INTERRUPT_HR	(1 << 3)	/* per hour   irq */
#define FTRTC010_CR_INTERRUPT_DAY	(1 << 4)	/* per day    irq */

static struct ftrtc010 *rtc = (struct ftrtc010 *)CONFIG_FTRTC010_BASE;

static void ftrtc010_enable(void)
{
	writel(FTRTC010_CR_ENABLE, &rtc->cr);
}

/*
 * return current time in seconds
 */
static unsigned long ftrtc010_time(void)
{
	unsigned long day;
	unsigned long hour;
	unsigned long minute;
	unsigned long second;
	unsigned long second2;

	do {
		second	= readl(&rtc->sec);
		day	= readl(&rtc->day);
		hour	= readl(&rtc->hour);
		minute	= readl(&rtc->min);
		second2	= readl(&rtc->sec);
	} while (second != second2);

	return day * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second;
}

/*
 * Get the current time from the RTC
 */

int rtc_get(struct rtc_time *tmp)
{
	unsigned long now;

	debug("%s(): record register: %x\n",
	      __func__, readl(&rtc->record));

	now = ftrtc010_time() + readl(&rtc->record);

	to_tm(now, tmp);

	return 0;
}

/*
 * Set the RTC
 */
int rtc_set(struct rtc_time *tmp)
{
	unsigned long new;
	unsigned long now;

	debug("%s(): DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	      __func__,
	      tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	new = mktime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_hour,
		     tmp->tm_min, tmp->tm_sec);

	now = ftrtc010_time();

	debug("%s(): write %lx to record register\n", __func__, new - now);

	writel(new - now, &rtc->record);

	return 0;
}

void rtc_reset(void)
{
	debug("%s()\n", __func__);
	ftrtc010_enable();
}

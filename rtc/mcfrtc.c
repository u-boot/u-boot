/*
 * Copyright (C) 2004-2007 Freescale Semiconductor, Inc.
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
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

#if defined(CONFIG_MCFRTC) && defined(CONFIG_CMD_DATE)

#include <command.h>
#include <rtc.h>
#include <asm/immap.h>
#include <asm/rtc.h>

#undef RTC_DEBUG

#ifndef CFG_MCFRTC_BASE
#error RTC_BASE is not defined!
#endif

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)
#define	STARTOFTIME		1970

void rtc_get(struct rtc_time *tmp)
{
	volatile rtc_t *rtc = (rtc_t *) (CFG_MCFRTC_BASE);

	int rtc_days, rtc_hrs, rtc_mins;
	int tim;

	rtc_days = rtc->days;
	rtc_hrs = rtc->hourmin >> 8;
	rtc_mins = RTC_HOURMIN_MINUTES(rtc->hourmin);

	tim = (rtc_days * 24) + rtc_hrs;
	tim = (tim * 60) + rtc_mins;
	tim = (tim * 60) + rtc->seconds;

	to_tm(tim, tmp);

	tmp->tm_yday = 0;
	tmp->tm_isdst = 0;

#ifdef RTC_DEBUG
	printf("Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif
}

void rtc_set(struct rtc_time *tmp)
{
	volatile rtc_t *rtc = (rtc_t *) (CFG_MCFRTC_BASE);

	static int month_days[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};
	int days, i, months;

	if (tmp->tm_year > 2037) {
		printf("Unable to handle. Exceeding integer limitation!\n");
		tmp->tm_year = 2027;
	}
#ifdef RTC_DEBUG
	printf("Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
	       tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
	       tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	/* calculate days by years */
	for (i = STARTOFTIME, days = 0; i < tmp->tm_year; i++) {
		days += 365 + isleap(i);
	}

	/* calculate days by months */
	months = tmp->tm_mon - 1;
	for (i = 0; i < months; i++) {
		days += month_days[i];

		if (i == 1)
			days += isleap(i);
	}

	days += tmp->tm_mday - 1;

	rtc->days = days;
	rtc->hourmin = (tmp->tm_hour << 8) | tmp->tm_min;
	rtc->seconds = tmp->tm_sec;
}

void rtc_reset(void)
{
	volatile rtc_t *rtc = (rtc_t *) (CFG_MCFRTC_BASE);

	if ((rtc->cr & RTC_CR_EN) == 0) {
		printf("real-time-clock was stopped. Now starting...\n");
		rtc->cr |= RTC_CR_EN;
	}

	rtc->cr |= RTC_CR_SWR;
}

#endif				/* CONFIG_MCFRTC && CONFIG_CMD_DATE */

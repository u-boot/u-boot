// SPDX-License-Identifier: GPL-2.0
/*
 * rtc and date/time utility functions
 *
 * Copyright (C) 2005-06 Tower Technologies
 * Author: Alessandro Zummo <a.zummo@towertech.it>
 *
 * U-Boot rtc_time differs from Linux rtc_time:
 * - The year field takes the actual value, not year - 1900.
 * - January is month 1.
 */

#include <common.h>
#include <rtc.h>
#include <linux/math64.h>

static const unsigned char rtc_days_in_month[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)

/*
 * The number of days in the month.
 */
static int rtc_month_days(unsigned int month, unsigned int year)
{
	return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

/*
 * rtc_to_tm - Converts u64 to rtc_time.
 * Convert seconds since 01-01-1970 00:00:00 to Gregorian date.
 */
void rtc_to_tm(u64 time, struct rtc_time *tm)
{
	unsigned int month, year, secs, days;

	days = div_u64_rem(time, 86400, &secs);

	/* day of the week, 1970-01-01 was a Thursday */
	tm->tm_wday = (days + 4) % 7;

	year = 1970 + days / 365;
	days -= (year - 1970) * 365
		+ LEAPS_THRU_END_OF(year - 1)
		- LEAPS_THRU_END_OF(1970 - 1);
	while (days < 0) {
		year -= 1;
		days += 365 + is_leap_year(year);
	}
	tm->tm_year = year; /* Not year - 1900 */
	tm->tm_yday = days + 1;

	for (month = 0; month < 11; month++) {
		int newdays;

		newdays = days - rtc_month_days(month, year);
		if (newdays < 0)
			break;
		days = newdays;
	}
	tm->tm_mon = month + 1; /* January = 1 */
	tm->tm_mday = days + 1;

	tm->tm_hour = secs / 3600;
	secs -= tm->tm_hour * 3600;
	tm->tm_min = secs / 60;
	tm->tm_sec = secs - tm->tm_min * 60;

	/* Zero unused fields */
	tm->tm_isdst = 0;
}

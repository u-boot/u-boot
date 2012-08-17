/*
 * Copyright (C) 2008, Guennadi Liakhovetski <lg@denx.de>
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
#include <rtc.h>
#include <spi.h>
#include <pmic.h>
#include <fsl_pmic.h>

int rtc_get(struct rtc_time *rtc)
{
	u32 day1, day2, time;
	int tim, i = 0;
	struct pmic *p = get_pmic();
	int ret;

	do {
		ret = pmic_reg_read(p, REG_RTC_DAY, &day1);
		if (ret < 0)
			return -1;

		ret = pmic_reg_read(p, REG_RTC_TIME, &time);
		if (ret < 0)
			return -1;

		ret = pmic_reg_read(p, REG_RTC_DAY, &day2);
		if (ret < 0)
			return -1;

	} while (day1 != day2 && i++ < 3);

	tim = day1 * 86400 + time;

	to_tm(tim, rtc);

	rtc->tm_yday = 0;
	rtc->tm_isdst = 0;

	return 0;
}

int rtc_set(struct rtc_time *rtc)
{
	u32 time, day;
	struct pmic *p = get_pmic();

	time = mktime(rtc->tm_year, rtc->tm_mon, rtc->tm_mday,
		      rtc->tm_hour, rtc->tm_min, rtc->tm_sec);
	day = time / 86400;
	time %= 86400;

	pmic_reg_write(p, REG_RTC_DAY, day);
	pmic_reg_write(p, REG_RTC_TIME, time);

	return 0;
}

void rtc_reset(void)
{
}

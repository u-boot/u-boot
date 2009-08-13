/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
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

/*
 * S3C44B0 CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/hardware.h>
#include <rtc.h>

int rtc_get (struct rtc_time* tm)
{
	RTCCON |= 1;
	tm->tm_year  = bcd2bin(BCDYEAR);
	tm->tm_mon   = bcd2bin(BCDMON);
	tm->tm_wday   = bcd2bin(BCDDATE);
	tm->tm_mday   = bcd2bin(BCDDAY);
	tm->tm_hour  = bcd2bin(BCDHOUR);
	tm->tm_min  = bcd2bin(BCDMIN);
	tm->tm_sec  = bcd2bin(BCDSEC);

	if (tm->tm_sec==0) {
		/* we have to re-read the rtc data because of the "one second deviation" problem */
		/* see RTC datasheet for more info about it */
		tm->tm_year  = bcd2bin(BCDYEAR);
		tm->tm_mon   = bcd2bin(BCDMON);
		tm->tm_mday   = bcd2bin(BCDDAY);
		tm->tm_wday   = bcd2bin(BCDDATE);
		tm->tm_hour  = bcd2bin(BCDHOUR);
		tm->tm_min  = bcd2bin(BCDMIN);
		tm->tm_sec  = bcd2bin(BCDSEC);
	}

	RTCCON &= ~1;

	if(tm->tm_year >= 70)
		tm->tm_year += 1900;
	else
		tm->tm_year += 2000;

	return 0;
}

int rtc_set (struct rtc_time* tm)
{
	if(tm->tm_year < 2000)
		tm->tm_year -= 1900;
	else
		tm->tm_year -= 2000;

	RTCCON |= 1;
	BCDYEAR = bin2bcd(tm->tm_year);
	BCDMON = bin2bcd(tm->tm_mon);
	BCDDAY = bin2bcd(tm->tm_mday);
	BCDDATE = bin2bcd(tm->tm_wday);
	BCDHOUR = bin2bcd(tm->tm_hour);
	BCDMIN = bin2bcd(tm->tm_min);
	BCDSEC = bin2bcd(tm->tm_sec);
	RTCCON &= 1;

	return 0;
}

void rtc_reset (void)
{
	RTCCON |= 1;
	BCDYEAR = 0;
	BCDMON = 0;
	BCDDAY = 0;
	BCDDATE = 0;
	BCDHOUR = 0;
	BCDMIN = 0;
	BCDSEC = 0;
	RTCCON &= 1;
}

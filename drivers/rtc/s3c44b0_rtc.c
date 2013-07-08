/*
 * (C) Copyright 2004
 * DAVE Srl
 * http://www.dave-tech.it
 * http://www.wawnet.biz
 * mailto:info@wawnet.biz
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

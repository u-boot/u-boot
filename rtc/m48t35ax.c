/*
 * (C) Copyright 2001
 * Erik Theisen,  Wave 7 Optics, etheisen@mindspring.com.
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
 * Date & Time support for ST Electronics M48T35Ax RTC
 */

/*#define       DEBUG */


#include <common.h>
#include <command.h>
#include <rtc.h>
#include <config.h>

#if defined(CONFIG_RTC_M48T35A) && (CONFIG_COMMANDS & CFG_CMD_DATE)

static uchar rtc_read  (uchar reg);
static void  rtc_write (uchar reg, uchar val);
static uchar bin2bcd   (unsigned int n);
static unsigned bcd2bin(uchar c);

/* ------------------------------------------------------------------------- */

void rtc_get (struct rtc_time *tmp)
{
	uchar sec, min, hour, cent_day, date, month, year;
	uchar ccr;			/* Clock control register */

	/* Lock RTC for read using clock control register */
	ccr = rtc_read(0);
	ccr = ccr | 0x40;
	rtc_write(0, ccr);

	sec	= rtc_read (0x1);
	min	= rtc_read (0x2);
	hour	= rtc_read (0x3);
	cent_day= rtc_read (0x4);
	date	= rtc_read (0x5);
	month   = rtc_read (0x6);
	year	= rtc_read (0x7);

	/* UNLock RTC */
	ccr = rtc_read(0);
	ccr = ccr & 0xBF;
	rtc_write(0, ccr);

	debug ( "Get RTC year: %02x month: %02x date: %02x cent_day: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, month, date, cent_day,
		hour, min, sec );

	tmp->tm_sec  = bcd2bin (sec  & 0x7F);
	tmp->tm_min  = bcd2bin (min  & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (date & 0x3F);
	tmp->tm_mon  = bcd2bin (month & 0x1F);
	tmp->tm_year = bcd2bin (year) + ((cent_day & 0x10) ? 2000 : 1900);
	tmp->tm_wday = bcd2bin (cent_day & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

void rtc_set (struct rtc_time *tmp)
{
	uchar ccr;			/* Clock control register */
	uchar century;

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	/* Lock RTC for write using clock control register */
	ccr = rtc_read(0);
	ccr = ccr | 0x80;
	rtc_write(0, ccr);

	rtc_write (0x07, bin2bcd(tmp->tm_year % 100));
	rtc_write (0x06, bin2bcd(tmp->tm_mon));
	rtc_write (0x05, bin2bcd(tmp->tm_mday));

	century = ((tmp->tm_year >= 2000) ? 0x10 : 0) | 0x20;
	rtc_write (0x04, bin2bcd(tmp->tm_wday) | century);

	rtc_write (0x03, bin2bcd(tmp->tm_hour));
	rtc_write (0x02, bin2bcd(tmp->tm_min ));
	rtc_write (0x01, bin2bcd(tmp->tm_sec ));

	/* UNLock RTC */
	ccr = rtc_read(0);
	ccr = ccr & 0x7F;
	rtc_write(0, ccr);
}

void rtc_reset (void)
{
	uchar val;

	/* Clear all clock control registers */
	rtc_write (0x0, 0x80);		/* No Read Lock or calibration */

	/* Clear stop bit */
	val = rtc_read (0x1);
	val &= 0x7f;
	rtc_write(0x1, val);

	/* Enable century / disable frequency test */
	val = rtc_read (0x4);
	val = (val & 0xBF) | 0x20;
	rtc_write(0x4, val);

	/* Clear write lock */
	rtc_write(0x0, 0);
}

/* ------------------------------------------------------------------------- */

static uchar rtc_read (uchar reg)
{
	uchar val;
	val = *(unsigned char *)
		((CFG_NVRAM_BASE_ADDR + CFG_NVRAM_SIZE - 8) + reg);
	return val;
}

static void rtc_write (uchar reg, uchar val)
{
	*(unsigned char *)
		((CFG_NVRAM_BASE_ADDR + CFG_NVRAM_SIZE - 8) + reg) = val;
}

static unsigned bcd2bin (uchar n)
{
	return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

static unsigned char bin2bcd (unsigned int n)
{
	return (((n / 10) << 4) | (n % 10));
}

#endif	/* CONFIG_RTC_M48T35A && CFG_CMD_DATE */

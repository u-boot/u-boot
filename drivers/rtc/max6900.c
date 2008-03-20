/*
 * (C) Copyright 2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * Date & Time support for MAXIM MAX6900 RTC
 */

/* #define	DEBUG	*/

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <i2c.h>

#if defined(CONFIG_RTC_MAX6900) && defined(CONFIG_CMD_DATE)

#ifndef	CFG_I2C_RTC_ADDR
#define	CFG_I2C_RTC_ADDR	0x50
#endif

/* ------------------------------------------------------------------------- */

static uchar rtc_read (uchar reg)
{
	return (i2c_reg_read (CFG_I2C_RTC_ADDR, reg));
}

static void rtc_write (uchar reg, uchar val)
{
	i2c_reg_write (CFG_I2C_RTC_ADDR, reg, val);
	udelay(2500);
}

static unsigned bcd2bin (uchar n)
{
	return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

static unsigned char bin2bcd (unsigned int n)
{
	return (((n / 10) << 4) | (n % 10));
}

/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, wday, mon, cent, year;
	int retry = 1;

	do {
		sec	= rtc_read (0x80);
		min	= rtc_read (0x82);
		hour	= rtc_read (0x84);
		mday	= rtc_read (0x86);
		mon	= rtc_read (0x88);
		wday	= rtc_read (0x8a);
		year	= rtc_read (0x8c);
		cent	= rtc_read (0x92);
		/*
		 * Check for seconds rollover
		 */
		if ((sec != 59) || (rtc_read(0x80) == sec)){
			retry = 0;
		}
	} while (retry);

	debug ( "Get RTC year: %02x mon: %02x cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon, cent, mday, wday,
		hour, min, sec );

	tmp->tm_sec  = bcd2bin (sec  & 0x7F);
	tmp->tm_min  = bcd2bin (min  & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year) + bcd2bin(cent) * 100;
	tmp->tm_wday = bcd2bin (wday & 0x07);
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

void rtc_set (struct rtc_time *tmp)
{

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	rtc_write (0x9E, 0x00);
	rtc_write (0x80, 0);	/* Clear seconds to ensure no rollover */
	rtc_write (0x92, bin2bcd(tmp->tm_year / 100));
	rtc_write (0x8c, bin2bcd(tmp->tm_year % 100));
	rtc_write (0x8a, bin2bcd(tmp->tm_wday));
	rtc_write (0x88, bin2bcd(tmp->tm_mon));
	rtc_write (0x86, bin2bcd(tmp->tm_mday));
	rtc_write (0x84, bin2bcd(tmp->tm_hour));
	rtc_write (0x82, bin2bcd(tmp->tm_min ));
	rtc_write (0x80, bin2bcd(tmp->tm_sec ));
}

void rtc_reset (void)
{
}

#endif

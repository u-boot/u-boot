/*
 * (C) Copyright 2001
 * Denis Peter MPL AG Switzerland. d.peter@mpl.ch
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
 * Date & Time support for the MC146818 (PIXX4) RTC
 */

/*#define	DEBUG*/

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)

static uchar rtc_read  (uchar reg);
static void  rtc_write (uchar reg, uchar val);
static uchar bin2bcd   (unsigned int n);
static unsigned bcd2bin(uchar c);

#define RTC_PORT_MC146818	CONFIG_SYS_ISA_IO_BASE_ADDRESS +  0x70
#define RTC_SECONDS		0x00
#define RTC_SECONDS_ALARM	0x01
#define RTC_MINUTES		0x02
#define RTC_MINUTES_ALARM	0x03
#define RTC_HOURS		0x04
#define RTC_HOURS_ALARM		0x05
#define RTC_DAY_OF_WEEK		0x06
#define RTC_DATE_OF_MONTH	0x07
#define RTC_MONTH		0x08
#define RTC_YEAR		0x09
#define RTC_CONFIG_A		0x0A
#define RTC_CONFIG_B		0x0B
#define RTC_CONFIG_C		0x0C
#define RTC_CONFIG_D		0x0D


/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, wday, mon, year;
  /* here check if rtc can be accessed */
	while((rtc_read(RTC_CONFIG_A)&0x80)==0x80);
	sec	= rtc_read (RTC_SECONDS);
	min	= rtc_read (RTC_MINUTES);
	hour	= rtc_read (RTC_HOURS);
	mday	= rtc_read (RTC_DATE_OF_MONTH);
	wday	= rtc_read (RTC_DAY_OF_WEEK);
	mon	= rtc_read (RTC_MONTH);
	year	= rtc_read (RTC_YEAR);
#ifdef CONFIG_AMIGAONEG3SE
	wday -= 1; /* VIA 686 stores Sunday = 1, Monday = 2, ... */
#endif
#ifdef RTC_DEBUG
	printf ( "Get RTC year: %02x mon/cent: %02x mday: %02x wday: %02x "
		"hr: %02x min: %02x sec: %02x\n",
		year, mon, mday, wday,
		hour, min, sec );
	printf ( "Alarms: month: %02x hour: %02x min: %02x sec: %02x\n",
		rtc_read (RTC_CONFIG_D) & 0x3F,
		rtc_read (RTC_HOURS_ALARM),
		rtc_read (RTC_MINUTES_ALARM),
		rtc_read (RTC_SECONDS_ALARM) );
#endif
	tmp->tm_sec  = bcd2bin (sec  & 0x7F);
	tmp->tm_min  = bcd2bin (min  & 0x7F);
	tmp->tm_hour = bcd2bin (hour & 0x3F);
	tmp->tm_mday = bcd2bin (mday & 0x3F);
	tmp->tm_mon  = bcd2bin (mon & 0x1F);
	tmp->tm_year = bcd2bin (year);
	tmp->tm_wday = bcd2bin (wday & 0x07);
	if(tmp->tm_year<70)
		tmp->tm_year+=2000;
	else
		tmp->tm_year+=1900;
	tmp->tm_yday = 0;
	tmp->tm_isdst= 0;
#ifdef RTC_DEBUG
	printf ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
#ifdef RTC_DEBUG
	printf ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif
	rtc_write(RTC_CONFIG_B,0x82); /* disables the RTC to update the regs */

	rtc_write (RTC_YEAR, bin2bcd(tmp->tm_year % 100));
	rtc_write (RTC_MONTH, bin2bcd(tmp->tm_mon));
#ifdef CONFIG_AMIGAONEG3SE
	rtc_write (RTC_DAY_OF_WEEK, bin2bcd(tmp->tm_wday)+1);
#else
	rtc_write (RTC_DAY_OF_WEEK, bin2bcd(tmp->tm_wday));
#endif
	rtc_write (RTC_DATE_OF_MONTH, bin2bcd(tmp->tm_mday));
	rtc_write (RTC_HOURS, bin2bcd(tmp->tm_hour));
	rtc_write (RTC_MINUTES, bin2bcd(tmp->tm_min ));
	rtc_write (RTC_SECONDS, bin2bcd(tmp->tm_sec ));
	rtc_write(RTC_CONFIG_B,0x02); /* enables the RTC to update the regs */

	return 0;
}

void rtc_reset (void)
{
	rtc_write(RTC_CONFIG_B,0x82); /* disables the RTC to update the regs */
	rtc_write(RTC_CONFIG_A,0x20); /* Normal OP */
	rtc_write(RTC_CONFIG_B,0x00);
	rtc_write(RTC_CONFIG_B,0x00);
	rtc_write(RTC_CONFIG_B,0x02); /* enables the RTC to update the regs */
}

/* ------------------------------------------------------------------------- */

#ifdef CONFIG_SYS_RTC_REG_BASE_ADDR
/*
 * use direct memory access
 */
static uchar rtc_read (uchar reg)
{
	return(in8(CONFIG_SYS_RTC_REG_BASE_ADDR+reg));
}

static void rtc_write (uchar reg, uchar val)
{
	out8(CONFIG_SYS_RTC_REG_BASE_ADDR+reg, val);
}
#else
static uchar rtc_read (uchar reg)
{
	out8(RTC_PORT_MC146818,reg);
	return(in8(RTC_PORT_MC146818+1));
}

static void rtc_write (uchar reg, uchar val)
{
	out8(RTC_PORT_MC146818,reg);
	out8(RTC_PORT_MC146818+1,val);
}
#endif

static unsigned bcd2bin (uchar n)
{
	return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

static unsigned char bin2bcd (unsigned int n)
{
	return (((n / 10) << 4) | (n % 10));
}

#endif

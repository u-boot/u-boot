/*
 * (C) Copyright 2003
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
 * Date & Time support for the DS12887 RTC
 */

#undef	RTC_DEBUG

#include <common.h>
#include <command.h>
#include <config.h>
#include <rtc.h>

#if defined(CONFIG_RTC_DS12887) && (CONFIG_COMMANDS & CFG_CMD_DATE)

#define RTC_SECONDS			0x00
#define RTC_SECONDS_ALARM		0x01
#define RTC_MINUTES			0x02
#define RTC_MINUTES_ALARM		0x03
#define RTC_HOURS			0x04
#define RTC_HOURS_ALARM 		0x05
#define RTC_DAY_OF_WEEK 		0x06
#define RTC_DATE_OF_MONTH		0x07
#define RTC_MONTH			0x08
#define RTC_YEAR			0x09
#define RTC_CONTROL_A 			0x0A
#define RTC_CONTROL_B 			0x0B
#define RTC_CONTROL_C 			0x0C
#define RTC_CONTROL_D			0x0D

#define RTC_CA_UIP			0x80
#define RTC_CB_DM			0x04
#define RTC_CB_24_12			0x02
#define RTC_CB_SET			0x80

#if defined(CONFIG_ATC)

static uchar rtc_read (uchar reg)
{
	uchar val;

	*(volatile unsigned char*)(RTC_PORT_ADDR) = reg;
	__asm__ __volatile__ ("sync");

	val = *(volatile unsigned char*)(RTC_PORT_DATA);
	return (val);
}

static void rtc_write (uchar reg, uchar val)
{
	*(volatile unsigned char*)(RTC_PORT_ADDR) = reg;
	__asm__ __volatile__ ("sync");

	*(volatile unsigned char*)(RTC_PORT_DATA) = val;
	__asm__ __volatile__ ("sync");
}

#else
# error Board specific rtc access functions should be supplied
#endif

static unsigned bcd2bin (uchar n)
{
	return ((((n >> 4) & 0x0F) * 10) + (n & 0x0F));
}

static unsigned char bin2bcd (unsigned int n)
{
	return (((n / 10) << 4) | (n % 10));
}

/* ------------------------------------------------------------------------- */

void rtc_get (struct rtc_time *tmp)
{
	uchar sec, min, hour, mday, wday, mon, year;

	/* check if rtc is available for access */
	while( rtc_read(RTC_CONTROL_A) & RTC_CA_UIP)
		;

	sec  = rtc_read(RTC_SECONDS);
	min  = rtc_read(RTC_MINUTES);
	hour = rtc_read(RTC_HOURS);
	mday = rtc_read(RTC_DATE_OF_MONTH);
	wday = rtc_read(RTC_DAY_OF_WEEK);
	mon  = rtc_read(RTC_MONTH);
	year = rtc_read(RTC_YEAR);

#ifdef RTC_DEBUG
	printf( "Get RTC year: %d; mon: %d; mday: %d; wday: %d; "
		"hr: %d; min: %d; sec: %d\n",
		year, mon, mday, wday, hour, min, sec );

	printf ( "Alarms: hour: %02x min: %02x sec: %02x\n",
		 rtc_read (RTC_HOURS_ALARM),
		 rtc_read (RTC_MINUTES_ALARM),
		 rtc_read (RTC_SECONDS_ALARM) );
#endif

	if( !(rtc_read(RTC_CONTROL_B) & RTC_CB_DM))
	{	    /* Information is in BCD format */
printf(" Get: Convert BSD to BIN\n");
		tmp->tm_sec  = bcd2bin (sec  & 0x7F);
		tmp->tm_min  = bcd2bin (min  & 0x7F);
		tmp->tm_hour = bcd2bin (hour & 0x3F);
		tmp->tm_mday = bcd2bin (mday & 0x3F);
		tmp->tm_mon  = bcd2bin (mon & 0x1F);
		tmp->tm_year = bcd2bin (year);
		tmp->tm_wday = bcd2bin (wday & 0x07);
	}
else
	{
		tmp->tm_sec  = sec  & 0x7F;
		tmp->tm_min  = min  & 0x7F;
		tmp->tm_hour = hour & 0x3F;
		tmp->tm_mday = mday & 0x3F;
		tmp->tm_mon  = mon & 0x1F;
		tmp->tm_year = year;
		tmp->tm_wday = wday & 0x07;
	}


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
}

void rtc_set (struct rtc_time *tmp)
{
	uchar save_ctrl_b;
	uchar sec, min, hour, mday, wday, mon, year;

#ifdef RTC_DEBUG
	printf ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
#endif

	if( !(rtc_read(RTC_CONTROL_B) & RTC_CB_DM))
	{	    /* Information is in BCD format */
		year = bin2bcd(tmp->tm_year % 100);
		mon  = bin2bcd(tmp->tm_mon);
		wday = bin2bcd(tmp->tm_wday);
		mday = bin2bcd(tmp->tm_mday);
		hour = bin2bcd(tmp->tm_hour);
		min  = bin2bcd(tmp->tm_min);
		sec  = bin2bcd(tmp->tm_sec);
	}
	else
	{
		year = tmp->tm_year % 100;
		mon  = tmp->tm_mon;
		wday = tmp->tm_wday;
		mday = tmp->tm_mday;
		hour = tmp->tm_hour;
		min  = tmp->tm_min;
		sec  = tmp->tm_sec;
	}

	/* disables the RTC to update the regs */
	save_ctrl_b = rtc_read(RTC_CONTROL_B);
	save_ctrl_b |= RTC_CB_SET;
	rtc_write(RTC_CONTROL_B, save_ctrl_b);

	rtc_write (RTC_YEAR, year);
	rtc_write (RTC_MONTH, mon);
	rtc_write (RTC_DAY_OF_WEEK, wday);
	rtc_write (RTC_DATE_OF_MONTH, mday);
	rtc_write (RTC_HOURS, hour);
	rtc_write (RTC_MINUTES, min);
	rtc_write (RTC_SECONDS, sec);

	/* enables the RTC to update the regs */
	save_ctrl_b &= ~RTC_CB_SET;
	rtc_write(RTC_CONTROL_B, save_ctrl_b);
}

void rtc_reset (void)
{
	struct rtc_time tmp;
	uchar ctrl_rg;

	ctrl_rg = RTC_CB_SET;
	rtc_write(RTC_CONTROL_B,ctrl_rg);

	tmp.tm_year = 1970 % 100;
	tmp.tm_mon = 1;
	tmp.tm_mday= 1;
	tmp.tm_hour = 0;
	tmp.tm_min = 0;
	tmp.tm_sec = 0;

#ifdef RTC_DEBUG
	printf ( "RTC:   %4d-%02d-%02d %2d:%02d:%02d UTC\n",
		    tmp.tm_year, tmp.tm_mon, tmp.tm_mday,
		    tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
#endif

	ctrl_rg = RTC_CB_SET | RTC_CB_24_12 | RTC_CB_DM;
	rtc_write(RTC_CONTROL_B,ctrl_rg);
	rtc_set(&tmp);

	rtc_write(RTC_HOURS_ALARM, 0),
	rtc_write(RTC_MINUTES_ALARM, 0),
	rtc_write(RTC_SECONDS_ALARM, 0);

	ctrl_rg = RTC_CB_24_12 | RTC_CB_DM;
	rtc_write(RTC_CONTROL_B,ctrl_rg);
}

#endif  /* (CONFIG_RTC_DS12887) && (CONFIG_COMMANDS & CFG_CMD_DATE) */

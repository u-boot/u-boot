/*
 * (C) Copyright 2004
 * Reinhard Meyer, EMK Elektronik GmbH
 * r.meyer@emk-elektronik.de
 * www.emk-elektronik.de
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

/*****************************************************************************
 * Date & Time support for internal RTC of MPC52xx
 *****************************************************************************/
/*#define	DEBUG*/

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)

/*****************************************************************************
 * this structure should be defined in mpc5200.h ...
 *****************************************************************************/
typedef struct rtc5200 {
	volatile ulong	tsr;	/* MBAR+0x800: time set register */
	volatile ulong	dsr;	/* MBAR+0x804: data set register */
	volatile ulong	nysr;	/* MBAR+0x808: new year and stopwatch register */
	volatile ulong	aier;	/* MBAR+0x80C: alarm and interrupt enable register */
	volatile ulong	ctr;	/* MBAR+0x810: current time register */
	volatile ulong	cdr;	/* MBAR+0x814: current data register */
	volatile ulong	asir;	/* MBAR+0x818: alarm and stopwatch interrupt register */
	volatile ulong	piber;	/* MBAR+0x81C: periodic interrupt and bus error register */
	volatile ulong	trdr;	/* MBAR+0x820: test register/divides register */
} RTC5200;

#define	RTC_SET		0x02000000
#define	RTC_PAUSE	0x01000000

/*****************************************************************************
 * get time
 *****************************************************************************/
int rtc_get (struct rtc_time *tmp)
{
	RTC5200	*rtc = (RTC5200 *) (CONFIG_SYS_MBAR+0x800);
	ulong time, date, time2;

	/* read twice to avoid getting a funny time when the second is just changing */
	do {
		time = rtc->ctr;
		date = rtc->cdr;
		time2 = rtc->ctr;
	} while (time != time2);

	tmp->tm_year	= date & 0xfff;
	tmp->tm_mon		= (date >> 24) & 0xf;
	tmp->tm_mday	= (date >> 16) & 0x1f;
	tmp->tm_wday	= (date >> 21) & 7;
	/* sunday is 7 in 5200 but 0 in rtc_time */
	if (tmp->tm_wday == 7)
		tmp->tm_wday = 0;
	tmp->tm_hour	= (time >> 16) & 0x1f;
	tmp->tm_min		= (time >> 8) & 0x3f;
	tmp->tm_sec		= time & 0x3f;

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

/*****************************************************************************
 * set time
 *****************************************************************************/
int rtc_set (struct rtc_time *tmp)
{
	RTC5200	*rtc = (RTC5200 *) (CONFIG_SYS_MBAR+0x800);
	ulong time, date, year;

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	time = (tmp->tm_hour << 16) | (tmp->tm_min << 8) | tmp->tm_sec;
	date = (tmp->tm_mon << 16) | tmp->tm_mday;
	if (tmp->tm_wday == 0)
		date |= (7 << 8);
	else
		date |= (tmp->tm_wday << 8);
	year = tmp->tm_year;

	/* mask unwanted bits that might show up when rtc_time is corrupt */
	time &= 0x001f3f3f;
	date &= 0x001f071f;
	year &= 0x00000fff;

	/* pause and set the RTC */
	rtc->nysr = year;
	rtc->dsr = date | RTC_PAUSE;
	udelay (1000);
	rtc->dsr = date | RTC_PAUSE | RTC_SET;
	udelay (1000);
	rtc->dsr = date | RTC_PAUSE;
	udelay (1000);
	rtc->dsr = date;
	udelay (1000);

	rtc->tsr = time | RTC_PAUSE;
	udelay (1000);
	rtc->tsr = time | RTC_PAUSE | RTC_SET;
	udelay (1000);
	rtc->tsr = time | RTC_PAUSE;
	udelay (1000);
	rtc->tsr = time;
	udelay (1000);

	return 0;
}

/*****************************************************************************
 * reset rtc circuit
 *****************************************************************************/
void rtc_reset (void)
{
	return;	/* nothing to do */
}

#endif

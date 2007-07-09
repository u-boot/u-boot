/*
 * (C) Copyright 2001
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
 * Real Time Clock interface of ADI21535 (Blackfin) for uCLinux
 *
 * Copyright (C) 2003 Motorola Corporation.  All rights reserved.
 * 				Richard Xiao (A2590C@email.mot.com)
 *
 * Copyright (C) 1996 Paul Gortmaker
 *
 *
 *	Based on other minimal char device drivers, like Alan's
 *	watchdog, Ted's random, etc. etc.
 *
 *	1.07	Paul Gortmaker.
 *	1.08	Miquel van Smoorenburg: disallow certain things on the
 *		DEC Alpha as the CMOS clock is also used for other things.
 *	1.09	Nikita Schmidt: epoch support and some Alpha cleanup.
 *	1.09a	Pete Zaitcev: Sun SPARC
 *	1.09b	Jeff Garzik: Modularize, init cleanup
 *	1.09c	Jeff Garzik: SMP cleanup
 *	1.10    Paul Barton-Davis: add support for async I/O
 *	1.10a	Andrea Arcangeli: Alpha updates
 *	1.10b	Andrew Morton: SMP lock fix
 *	1.10c	Cesar Barros: SMP locking fixes and cleanup
 *	1.10d	Paul Gortmaker: delete paranoia check in rtc_exit
 *	1.10e   LG Soft India: Register access is different in BF533.
 */

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_RTC_BFIN) && defined(CONFIG_CMD_DATE)

#include <asm/blackfin.h>
#include <asm/arch/bf5xx_rtc.h>

void rtc_reset(void)
{
	return;			/* nothing to do */
}

/* Wait for pending writes to complete */
void wait_for_complete(void)
{
	while (!(*(volatile unsigned short *)RTC_ISTAT & 0x8000)) {
		printf("");
	}
	*(volatile unsigned short *)RTC_ISTAT = 0x8000;
}

/* Enable the RTC prescaler enable register */
void rtc_init()
{
	*(volatile unsigned short *)RTC_PREN = 0x1;
	wait_for_complete();
}

/* Set the time. Get the time_in_secs which is the number of seconds since Jan 1970 and set the RTC registers
 * based on this value.
 */
void rtc_set(struct rtc_time *tmp)
{
	unsigned long n_days_1970 = 0;
	unsigned long n_secs_rem = 0;
	unsigned long n_hrs = 0;
	unsigned long n_mins = 0;
	unsigned long n_secs = 0;
	unsigned long time_in_secs;

	if (tmp == NULL) {
		printf("Error setting the date/time \n");
		return;
	}

	time_in_secs =
	    mktime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_hour,
		   tmp->tm_min, tmp->tm_sec);

	/* Compute no. of days since 1970 */
	n_days_1970 = (unsigned long)(time_in_secs / (NUM_SECS_IN_DAY));

	/* From the remining secs, compute the hrs(0-23), mins(0-59) and secs(0-59) */
	n_secs_rem = (unsigned long)(time_in_secs % (NUM_SECS_IN_DAY));
	n_hrs = n_secs_rem / (NUM_SECS_IN_HOUR);
	n_secs_rem = n_secs_rem % (NUM_SECS_IN_HOUR);
	n_mins = n_secs_rem / (NUM_SECS_IN_MIN);
	n_secs = n_secs_rem % (NUM_SECS_IN_MIN);

	/* Store the new time in the RTC_STAT register */
	*(volatile unsigned long *)RTC_STAT =
	    ((n_days_1970 << DAY_BITS_OFF) | (n_hrs << HOUR_BITS_OFF) |
	     (n_mins << MIN_BITS_OFF) | (n_secs << SEC_BITS_OFF));

	wait_for_complete();
}

/* Read the time from the RTC_STAT. time_in_seconds is seconds since Jan 1970 */
void rtc_get(struct rtc_time *tmp)
{
	unsigned long cur_rtc_stat = 0;
	unsigned long time_in_sec;
	unsigned long tm_sec = 0, tm_min = 0, tm_hour = 0, tm_day = 0;

	if (tmp == NULL) {
		printf("Error getting the date/time \n");
		return;
	}

	/* Read the RTC_STAT register */
	cur_rtc_stat = *(volatile unsigned long *)RTC_STAT;

	/* Get the secs (0-59), mins (0-59), hrs (0-23) and the days since Jan 1970 */
	tm_sec = (cur_rtc_stat >> SEC_BITS_OFF) & 0x3f;
	tm_min = (cur_rtc_stat >> MIN_BITS_OFF) & 0x3f;
	tm_hour = (cur_rtc_stat >> HOUR_BITS_OFF) & 0x1f;
	tm_day = (cur_rtc_stat >> DAY_BITS_OFF) & 0x7fff;

	/* Calculate the total number of seconds since Jan 1970 */
	time_in_sec = (tm_sec) +
	    MIN_TO_SECS(tm_min) + HRS_TO_SECS(tm_hour) + DAYS_TO_SECS(tm_day);
	to_tm(time_in_sec, tmp);
}
#endif				/* CONFIG_RTC_BFIN && CFG_CMD_DATE */

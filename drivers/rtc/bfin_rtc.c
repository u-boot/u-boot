/*
 * Copyright (c) 2004-2008 Analog Devices Inc.
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)

#include <asm/blackfin.h>
#include <asm/mach-common/bits/rtc.h>

#define pr_stamp() debug("%s:%s:%i: here i am\n", __FILE__, __func__, __LINE__)

#define MIN_TO_SECS(x)    (60 * (x))
#define HRS_TO_SECS(x)    (60 * MIN_TO_SECS(x))
#define DAYS_TO_SECS(x)   (24 * HRS_TO_SECS(x))

#define NUM_SECS_IN_MIN   MIN_TO_SECS(1)
#define NUM_SECS_IN_HR    HRS_TO_SECS(1)
#define NUM_SECS_IN_DAY   DAYS_TO_SECS(1)

/* Enable the RTC prescaler enable register */
static void rtc_init(void)
{
	if (!(bfin_read_RTC_PREN() & 0x1))
		bfin_write_RTC_PREN(0x1);
}

/* Our on-chip RTC has no notion of "reset" */
void rtc_reset(void)
{
	rtc_init();
}

/* Wait for pending writes to complete */
static void wait_for_complete(void)
{
	pr_stamp();
	while (!(bfin_read_RTC_ISTAT() & WRITE_COMPLETE))
		if (!(bfin_read_RTC_ISTAT() & WRITE_PENDING))
			break;
	bfin_write_RTC_ISTAT(WRITE_COMPLETE);
}

/* Set the time. Get the time_in_secs which is the number of seconds since Jan 1970 and set the RTC registers
 * based on this value.
 */
int rtc_set(struct rtc_time *tmp)
{
	unsigned long remain, days, hrs, mins, secs;

	pr_stamp();

	if (tmp == NULL) {
		puts("Error setting the date/time\n");
		return -1;
	}

	rtc_init();
	wait_for_complete();

	/* Calculate number of seconds this incoming time represents */
	remain = mktime(tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
	                tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	/* Figure out how many days since epoch */
	days = remain / NUM_SECS_IN_DAY;

	/* From the remaining secs, compute the hrs(0-23), mins(0-59) and secs(0-59) */
	remain = remain % NUM_SECS_IN_DAY;
	hrs = remain / NUM_SECS_IN_HR;
	remain = remain % NUM_SECS_IN_HR;
	mins = remain / NUM_SECS_IN_MIN;
	secs = remain % NUM_SECS_IN_MIN;

	/* Encode these time values into our RTC_STAT register */
	bfin_write_RTC_STAT(SET_ALARM(days, hrs, mins, secs));

	return 0;
}

/* Read the time from the RTC_STAT. time_in_seconds is seconds since Jan 1970 */
int rtc_get(struct rtc_time *tmp)
{
	uint32_t cur_rtc_stat;
	int time_in_sec;
	int tm_sec, tm_min, tm_hr, tm_day;

	pr_stamp();

	if (tmp == NULL) {
		puts("Error getting the date/time\n");
		return -1;
	}

	rtc_init();
	wait_for_complete();

	/* Read the RTC_STAT register */
	cur_rtc_stat = bfin_read_RTC_STAT();

	/* Convert our encoded format into actual time values */
	tm_sec = (cur_rtc_stat & RTC_SEC) >> RTC_SEC_P;
	tm_min = (cur_rtc_stat & RTC_MIN) >> RTC_MIN_P;
	tm_hr  = (cur_rtc_stat & RTC_HR ) >> RTC_HR_P;
	tm_day = (cur_rtc_stat & RTC_DAY) >> RTC_DAY_P;

	/* Calculate the total number of seconds since epoch */
	time_in_sec = (tm_sec) + MIN_TO_SECS(tm_min) + HRS_TO_SECS(tm_hr) + DAYS_TO_SECS(tm_day);
	to_tm(time_in_sec, tmp);

	return 0;
}

#endif

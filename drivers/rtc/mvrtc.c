/*
 * Copyright (C) 2011
 * Jason Cooper <u-boot@lakedaemon.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Date & Time support for Marvell Integrated RTC
 */

#include <common.h>
#include <command.h>
#include <rtc.h>
#include <asm/io.h>
#include "mvrtc.h"

/* This RTC does not support century, so we assume 20 */
#define CENTURY 20

int rtc_get(struct rtc_time *t)
{
	u32 time;
	u32 date;
	struct mvrtc_registers *mvrtc_regs;

	mvrtc_regs = (struct mvrtc_registers *)KW_RTC_BASE;

	/* read the time register */
	time = readl(&mvrtc_regs->time);

	/* read the date register */
	date = readl(&mvrtc_regs->date);

	/* test for 12 hour clock (can't tell if it's am/pm) */
	if (time & MVRTC_HRFMT_MSK) {
		printf("Error: RTC in 12 hour mode, can't determine AM/PM.\n");
		return -1;
	}

	/* time */
	t->tm_sec  = bcd2bin((time >> MVRTC_SEC_SFT)  & MVRTC_SEC_MSK);
	t->tm_min  = bcd2bin((time >> MVRTC_MIN_SFT)  & MVRTC_MIN_MSK);
	t->tm_hour = bcd2bin((time >> MVRTC_HOUR_SFT) & MVRTC_HOUR_MSK);
	t->tm_wday = bcd2bin((time >> MVRTC_DAY_SFT)  & MVRTC_DAY_MSK);
	t->tm_wday--;

	/* date */
	t->tm_mday = bcd2bin((date >> MVRTC_DATE_SFT) & MVRTC_DATE_MSK);
	t->tm_mon  = bcd2bin((date >> MVRTC_MON_SFT)  & MVRTC_MON_MSK);
	t->tm_year = bcd2bin((date >> MVRTC_YEAR_SFT) & MVRTC_YEAR_MSK);
	t->tm_year += CENTURY * 100;

	/* not supported in this RTC */
	t->tm_yday  = 0;
	t->tm_isdst = 0;

	return 0;
}

int rtc_set(struct rtc_time *t)
{
	u32 time = 0; /* sets hour format bit to zero, 24hr format. */
	u32 date = 0;
	struct mvrtc_registers *mvrtc_regs;

	mvrtc_regs = (struct mvrtc_registers *)KW_RTC_BASE;

	/* check that this code isn't 80+ years old ;-) */
	if ((t->tm_year / 100) != CENTURY)
		printf("Warning: Only century %d supported.\n", CENTURY);

	/* time */
	time |= (bin2bcd(t->tm_sec)      & MVRTC_SEC_MSK)  << MVRTC_SEC_SFT;
	time |= (bin2bcd(t->tm_min)      & MVRTC_MIN_MSK)  << MVRTC_MIN_SFT;
	time |= (bin2bcd(t->tm_hour)     & MVRTC_HOUR_MSK) << MVRTC_HOUR_SFT;
	time |= (bin2bcd(t->tm_wday + 1) & MVRTC_DAY_MSK)  << MVRTC_DAY_SFT;

	/* date */
	date |= (bin2bcd(t->tm_mday)       & MVRTC_DATE_MSK) << MVRTC_DATE_SFT;
	date |= (bin2bcd(t->tm_mon)        & MVRTC_MON_MSK)  << MVRTC_MON_SFT;
	date |= (bin2bcd(t->tm_year % 100) & MVRTC_YEAR_MSK) << MVRTC_YEAR_SFT;

	/* write the time register */
	writel(time, &mvrtc_regs->time);

	/* write the date register */
	writel(date, &mvrtc_regs->date);

	return 0;
}

void rtc_reset(void)
{
	u32 time;
	u32 sec;
	struct mvrtc_registers *mvrtc_regs;

	mvrtc_regs = (struct mvrtc_registers *)KW_RTC_BASE;

	/* no init routine for this RTC needed, just check that it's working */
	time = readl(&mvrtc_regs->time);
	sec  = bcd2bin((time >> MVRTC_SEC_SFT) & MVRTC_SEC_MSK);
	udelay(1000000);
	time = readl(&mvrtc_regs->time);

	if (sec == bcd2bin((time >> MVRTC_SEC_SFT) & MVRTC_SEC_MSK))
		printf("Error: RTC did not increment.\n");
}

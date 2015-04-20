/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Date & Time support for internal RTC of MPC8xx
 */

/*#define	DEBUG*/

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_CMD_DATE)

/* ------------------------------------------------------------------------- */

int rtc_get (struct rtc_time *tmp)
{
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	ulong tim;

	tim = immr->im_sit.sit_rtc;

	rtc_to_tm(tim, tmp);

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	return 0;
}

int rtc_set (struct rtc_time *tmp)
{
	volatile immap_t *immr = (immap_t *)CONFIG_SYS_IMMR;
	ulong tim;

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	tim = rtc_mktime(tmp);

	immr->im_sitk.sitk_rtck = KAPWR_KEY;
	immr->im_sit.sit_rtc = tim;

	return 0;
}

void rtc_reset (void)
{
	return;	/* nothing to do */
}

#endif

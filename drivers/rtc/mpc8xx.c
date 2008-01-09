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
 */

/*
 * Date & Time support for internal RTC of MPC8xx
 */

/*#define	DEBUG*/

#include <common.h>
#include <command.h>
#include <rtc.h>

#if defined(CONFIG_RTC_MPC8xx) && defined(CONFIG_CMD_DATE)

/* ------------------------------------------------------------------------- */

void rtc_get (struct rtc_time *tmp)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	ulong tim;

	tim = immr->im_sit.sit_rtc;

	to_tm (tim, tmp);

	debug ( "Get DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);
}

void rtc_set (struct rtc_time *tmp)
{
	volatile immap_t *immr = (immap_t *)CFG_IMMR;
	ulong tim;

	debug ( "Set DATE: %4d-%02d-%02d (wday=%d)  TIME: %2d:%02d:%02d\n",
		tmp->tm_year, tmp->tm_mon, tmp->tm_mday, tmp->tm_wday,
		tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	tim = mktime (tmp->tm_year, tmp->tm_mon, tmp->tm_mday,
		      tmp->tm_hour, tmp->tm_min, tmp->tm_sec);

	immr->im_sitk.sitk_rtck = KAPWR_KEY;
	immr->im_sit.sit_rtc = tim;
}

void rtc_reset (void)
{
	return;	/* nothing to do */
}

#endif

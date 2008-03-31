/*
 * (C) Copyright 2002
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

#include <common.h>

/*
 * RTC test
 *
 * The Real Time Clock (RTC) operation is verified by this test.
 * The following features are verified:
 *   o) Time uniformity
 *      This is verified by reading RTC in polling within
 *      a short period of time.
 *   o) Passing month boundaries
 *      This is checked by setting RTC to a second before
 *      a month boundary and reading it after its passing the
 *      boundary. The test is performed for both leap- and
 *      nonleap-years.
 */

#include <post.h>
#include <rtc.h>

#if CONFIG_POST & CFG_POST_RTC

static int rtc_post_skip (ulong * diff)
{
	struct rtc_time tm1;
	struct rtc_time tm2;
	ulong start1;
	ulong start2;

	rtc_get (&tm1);
	start1 = get_timer (0);

	while (1) {
		rtc_get (&tm2);
		start2 = get_timer (0);
		if (tm1.tm_sec != tm2.tm_sec)
			break;
		if (start2 - start1 > 1500)
			break;
	}

	if (tm1.tm_sec != tm2.tm_sec) {
		*diff = start2 - start1;

		return 0;
	} else {
		return -1;
	}
}

static void rtc_post_restore (struct rtc_time *tm, unsigned int sec)
{
	time_t t = mktime (tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_hour,
					   tm->tm_min, tm->tm_sec) + sec;
	struct rtc_time ntm;

	to_tm (t, &ntm);

	rtc_set (&ntm);
}

int rtc_post_test (int flags)
{
	ulong diff;
	unsigned int i;
	struct rtc_time svtm;
	static unsigned int daysnl[] =
			{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	static unsigned int daysl[] =
			{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned int ynl = 1999;
	unsigned int yl = 2000;
	unsigned int skipped = 0;

	/* Time uniformity */
	if (rtc_post_skip (&diff) != 0) {
		post_log ("Timeout while waiting for a new second !\n");

		return -1;
	}

	for (i = 0; i < 5; i++) {
		if (rtc_post_skip (&diff) != 0) {
			post_log ("Timeout while waiting for a new second !\n");

			return -1;
		}

		if (diff < 950 || diff > 1050) {
			post_log ("Invalid second duration !\n");

			return -1;
		}
	}

	/* Passing month boundaries */

	if (rtc_post_skip (&diff) != 0) {
		post_log ("Timeout while waiting for a new second !\n");

		return -1;
	}
	rtc_get (&svtm);

	for (i = 0; i < 12; i++) {
		time_t t = mktime (ynl, i + 1, daysnl[i], 23, 59, 59);
		struct rtc_time tm;

		to_tm (t, &tm);
		rtc_set (&tm);

		skipped++;
		if (rtc_post_skip (&diff) != 0) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Timeout while waiting for a new second !\n");

			return -1;
		}

		rtc_get (&tm);
		if (tm.tm_mon == i + 1) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Month %d boundary is not passed !\n", i + 1);

			return -1;
		}
	}

	for (i = 0; i < 12; i++) {
		time_t t = mktime (yl, i + 1, daysl[i], 23, 59, 59);
		struct rtc_time tm;

		to_tm (t, &tm);
		rtc_set (&tm);

		skipped++;
		if (rtc_post_skip (&diff) != 0) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Timeout while waiting for a new second !\n");

			return -1;
		}

		rtc_get (&tm);
		if (tm.tm_mon == i + 1) {
			rtc_post_restore (&svtm, skipped);
			post_log ("Month %d boundary is not passed !\n", i + 1);

			return -1;
		}
	}
	rtc_post_restore (&svtm, skipped);

	return 0;
}

#endif /* CONFIG_POST & CFG_POST_RTC */

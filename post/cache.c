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

/* Cache test
 *
 * This test verifies the CPU data and instruction cache using
 * several test scenarios.
 */

#ifdef CONFIG_POST

#include <post.h>
#include <watchdog.h>

#if CONFIG_POST & CFG_POST_CACHE

#define CACHE_POST_SIZE	1024

extern int cache_post_test1 (char *, unsigned int);
extern int cache_post_test2 (char *, unsigned int);
extern int cache_post_test3 (char *, unsigned int);
extern int cache_post_test4 (char *, unsigned int);
extern int cache_post_test5 (void);
extern int cache_post_test6 (void);

int cache_post_test (int flags)
{
	int ints = disable_interrupts ();
	int res = 0;
	static char ta[CACHE_POST_SIZE + 0xf];
	char *testarea = (char *) (((unsigned long) ta + 0xf) & ~0xf);

	WATCHDOG_RESET ();
	if (res == 0)
		res = cache_post_test1 (testarea, CACHE_POST_SIZE);
	WATCHDOG_RESET ();
	if (res == 0)
		res = cache_post_test2 (testarea, CACHE_POST_SIZE);
	WATCHDOG_RESET ();
	if (res == 0)
		res = cache_post_test3 (testarea, CACHE_POST_SIZE);
	WATCHDOG_RESET ();
	if (res == 0)
		res = cache_post_test4 (testarea, CACHE_POST_SIZE);
	WATCHDOG_RESET ();
	if (res == 0)
		res = cache_post_test5 ();
	WATCHDOG_RESET ();
	if (res == 0)
		res = cache_post_test6 ();

	WATCHDOG_RESET ();
	if (ints)
		enable_interrupts ();
	return res;
}

#endif /* CONFIG_POST & CFG_POST_CACHE */
#endif /* CONFIG_POST */

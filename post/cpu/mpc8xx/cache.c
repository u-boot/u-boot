/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/* Cache test
 *
 * This test verifies the CPU data and instruction cache using
 * several test scenarios.
 */

#include <post.h>
#include <watchdog.h>

#if CONFIG_POST & CONFIG_SYS_POST_CACHE

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

#endif /* CONFIG_POST & CONFIG_SYS_POST_CACHE */

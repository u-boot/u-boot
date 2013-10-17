/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Author: Igor Lisitsin <igor@emcraft.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

/*
 * Watchdog test
 *
 * The test verifies the watchdog timer operation.
 * On the first iteration, the test routine disables interrupts and
 * makes a 10-second delay. If the system does not reboot during this delay,
 * the watchdog timer is not operational and the test fails. If the system
 * reboots, on the second iteration the test routine reports a success.
 */

#include <post.h>

#if CONFIG_POST & CONFIG_SYS_POST_WATCHDOG

#include <watchdog.h>

int watchdog_post_test (int flags)
{
	if (flags & POST_REBOOT) {
		/* Test passed */
		return 0;
	}
	else {
		/* 10-second delay */
		int ints = disable_interrupts ();
		ulong base = post_time_ms (0);

		while (post_time_ms (base) < 10000)
			;
		if (ints)
			enable_interrupts ();

		/*
		 * If we have reached this point, the watchdog timer
		 * does not work
		 */
		return -1;
	}
}

#endif /* CONFIG_POST & CONFIG_SYS_POST_WATCHDOG */

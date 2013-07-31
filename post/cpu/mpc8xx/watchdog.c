/*
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <watchdog.h>

#if CONFIG_POST & CONFIG_SYS_POST_WATCHDOG

static ulong gettbl (void)
{
	ulong r;

  asm ("mftbl %0":"=r" (r));

	return r;
}

int watchdog_post_test (int flags)
{
	if (flags & POST_REBOOT) {
		/* Test passed */

		return 0;
	} else {
		/* 10-second delay */
		int ints = disable_interrupts ();
		ulong base = gettbl ();
		ulong clk = get_tbclk ();

		while ((gettbl () - base) / 10 < clk);

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

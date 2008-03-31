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

#if CONFIG_POST & CFG_POST_WATCHDOG

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

#endif /* CONFIG_POST & CFG_POST_WATCHDOG */

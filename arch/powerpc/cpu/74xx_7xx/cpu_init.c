/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * cpu_init.c - low level cpu init
 *
 * there's really nothing going on here yet.  future work area?
 */

#include <common.h>
#include <74xx_7xx.h>

/*
 * Breath some life into the CPU...
 *
 * there's basically nothing to do here since the memory controller
 * isn't on the CPU in this case.
 */
void
cpu_init_f (void)
{
	switch (get_cpu_type()) {
	case CPU_7450:
	case CPU_7455:
	case CPU_7457:
	case CPU_7447A:
	case CPU_7448:
		/* enable the timebase bit in HID0 */
		set_hid0(get_hid0() | 0x4000000);
		break;
	default:
		/* do nothing */
		break;
	}
}

/*
 * initialize higher level parts of CPU like timers
 */
int cpu_init_r (void)
{
	return (0);
}

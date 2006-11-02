/*
 * (C) Copyright 2001
 * Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
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

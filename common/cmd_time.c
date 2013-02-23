/*
 * Copyright (c) 2011 The Chromium OS Authors.
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
#include <command.h>

static void report_time(ulong cycles)
{
	ulong minutes, seconds, milliseconds;
	ulong total_seconds, remainder;

	total_seconds = cycles / CONFIG_SYS_HZ;
	remainder = cycles % CONFIG_SYS_HZ;
	minutes = total_seconds / 60;
	seconds = total_seconds % 60;
	/* approximate millisecond value */
	milliseconds = (remainder * 1000 + CONFIG_SYS_HZ / 2) / CONFIG_SYS_HZ;

	printf("\ntime:");
	if (minutes)
		printf(" %lu minutes,", minutes);
	printf(" %lu.%03lu seconds, %lu ticks\n",
			seconds, milliseconds, cycles);
}

static int do_time(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong cycles = 0;
	int retval = 0;
	int repeatable;

	if (argc == 1)
		return CMD_RET_USAGE;

	retval = cmd_process(0, argc - 1, argv + 1, &repeatable, &cycles);
	report_time(cycles);

	return retval;
}

U_BOOT_CMD(time, CONFIG_SYS_MAXARGS, 0, do_time,
		"run commands and summarize execution time",
		"command [args...]\n");

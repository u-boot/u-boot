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

/*
 * TODO(clchiou): This function actually minics the bottom-half of the
 * run_command() function.  Since this function has ARM-dependent timer
 * codes, we cannot merge it with the run_command() for now.
 */
static int run_command_and_time_it(int flag, int argc, char * const argv[],
		ulong *cycles)
{
	cmd_tbl_t *cmdtp = find_cmd(argv[0]);
	int retval = 0;

	if (!cmdtp) {
		printf("%s: command not found\n", argv[0]);
		return 1;
	}
	if (argc > cmdtp->maxargs)
		return CMD_RET_USAGE;

	/*
	 * TODO(clchiou): get_timer_masked() is only defined in certain ARM
	 * boards.  We could use the new timer API that Graeme is proposing
	 * so that this piece of code would be arch-independent.
	 */
	*cycles = get_timer_masked();
	retval = cmdtp->cmd(cmdtp, flag, argc, argv);
	*cycles = get_timer_masked() - *cycles;

	return retval;
}

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

	if (argc == 1)
		return CMD_RET_USAGE;

	retval = run_command_and_time_it(0, argc - 1, argv + 1, &cycles);
	report_time(cycles);

	return retval;
}

U_BOOT_CMD(time, CONFIG_SYS_MAXARGS, 0, do_time,
		"run commands and summarize execution time",
		"command [args...]\n");

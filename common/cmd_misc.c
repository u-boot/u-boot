/*
 * (C) Copyright 2001
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

/*
 * Misc functions
 */
#include <common.h>
#include <command.h>

static int do_sleep(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	ulong start = get_timer(0);
	ulong delay;

	if (argc != 2)
		return CMD_RET_USAGE;

	delay = simple_strtoul(argv[1], NULL, 10) * CONFIG_SYS_HZ;

	while (get_timer(start) < delay) {
		if (ctrlc())
			return (-1);

		udelay(100);
	}

	return 0;
}

U_BOOT_CMD(
	sleep ,    2,    1,     do_sleep,
	"delay execution for some time",
	"N\n"
	"    - delay execution for N seconds (N is _decimal_ !!!)"
);

#ifdef CONFIG_CMD_TIMER
static int do_timer(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	static ulong start;

	if (argc != 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "start"))
		start = get_timer(0);

	if (!strcmp(argv[1], "get")) {
		ulong msecs = get_timer(start) * 1000 / CONFIG_SYS_HZ;
		printf("%ld.%03d\n", msecs / 1000, (int)(msecs % 1000));
	}

	return 0;
}

U_BOOT_CMD(
	timer,    2,    1,     do_timer,
	"access the system timer",
	"start - Reset the timer reference.\n"
	"timer get   - Print the time since 'start'."
);
#endif

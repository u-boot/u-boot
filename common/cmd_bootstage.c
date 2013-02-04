/*
 * Copyright (c) 2012, Google Inc. All rights reserved.
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

#ifndef CONFIG_BOOTSTAGE_STASH
#define CONFIG_BOOTSTAGE_STASH		-1UL
#define CONFIG_BOOTSTAGE_STASH_SIZE	-1
#endif

static int do_bootstage_report(cmd_tbl_t *cmdtp, int flag, int argc,
			       char * const argv[])
{
	bootstage_report();

	return 0;
}

static int get_base_size(int argc, char * const argv[], ulong *basep,
			 ulong *sizep)
{
	char *endp;

	*basep = CONFIG_BOOTSTAGE_STASH;
	*sizep = CONFIG_BOOTSTAGE_STASH_SIZE;
	if (argc < 2)
		return 0;
	*basep = simple_strtoul(argv[1], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;
	if (argc == 2)
		return 0;
	*sizep = simple_strtoul(argv[2], &endp, 16);
	if (*argv[2] == 0 || *endp != 0)
		return -1;

	return 0;
}

static int do_bootstage_stash(cmd_tbl_t *cmdtp, int flag, int argc,
			      char * const argv[])
{
	ulong base, size;
	int ret;

	if (get_base_size(argc, argv, &base, &size))
		return CMD_RET_USAGE;
	if (base == -1UL) {
		printf("No bootstage stash area defined\n");
		return 1;
	}

	if (0 == strcmp(argv[0], "stash"))
		ret = bootstage_stash((void *)base, size);
	else
		ret = bootstage_unstash((void *)base, size);
	if (ret)
		return 1;

	return 0;
}

static cmd_tbl_t cmd_bootstage_sub[] = {
	U_BOOT_CMD_MKENT(report, 2, 1, do_bootstage_report, "", ""),
	U_BOOT_CMD_MKENT(stash, 4, 0, do_bootstage_stash, "", ""),
	U_BOOT_CMD_MKENT(unstash, 4, 0, do_bootstage_stash, "", ""),
};

/*
 * Process a bootstage sub-command
 */
static int do_boostage(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	cmd_tbl_t *c;

	/* Strip off leading 'bootstage' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], cmd_bootstage_sub,
			 ARRAY_SIZE(cmd_bootstage_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);
	else
		return CMD_RET_USAGE;
}


U_BOOT_CMD(bootstage, 4, 1, do_boostage,
	"Boot stage command",
	" - check boot progress and timing\n"
	"report                      - Print a report\n"
	"stash [<start> [<size>]]    - Stash data into memory\n"
	"unstash [<start> [<size>]]  - Unstash data from memory"
);

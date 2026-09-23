// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 * bootd - boot default, i.e. run the command in the "bootcmd" environment
 * variable
 */
#include <command.h>
#include <env.h>

int do_bootd(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	return run_command(env_get("bootcmd"), flag);
}

U_BOOT_CMD(
	boot,	1,	1,	do_bootd,
	"boot default, i.e., run 'bootcmd'",
	""
);

/* keep old command name "bootd" for backward compatibility */
U_BOOT_CMD(
	bootd, 1,	1,	do_bootd,
	"boot default, i.e., run 'bootcmd'",
	""
);

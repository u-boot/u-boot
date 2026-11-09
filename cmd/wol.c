// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018
 * Lothar Felte, lothar.felten@gmail.com
 */

/*
 * Wake-on-LAN support
 */
#include <command.h>
#include <net.h>
#include <net/wol.h>
#include <linux/ctype.h>

int do_wol(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	ulong timeout = 0;
	const char *p;

	/* Validate arguments */
	if (argc != 2 || !*argv[1])
		return CMD_RET_USAGE;
	for (p = argv[1]; *p; p++) {
		if (!isdigit(*p) || timeout > (ULONG_MAX / 1000 - (*p - '0')) / 10)
			return CMD_RET_USAGE;
		timeout = timeout * 10 + (*p - '0');
	}
	if (wol_wait(timeout * 1000) < 0)
		return CMD_RET_FAILURE;
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	wol,	2,	1,	do_wol,
	"wait for an incoming wake-on-lan packet",
	"Timeout"
);

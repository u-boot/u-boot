// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2025
 * Heiko Schocher, Nabladev Software Engineering, hs@nabladev.com
 *
 * based on code from cmd/md5sum.c
 */

#include <command.h>
#include <env.h>
#include <hash.h>

static int do_sm3sum(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int flags = HASH_FLAG_ENV;
	int ac;
	char *const *av;

	if (argc < 3)
		return CMD_RET_USAGE;

	av = argv + 1;
	ac = argc - 1;
	if (IS_ENABLED(CONFIG_SM3SUM_VERIFY) && strcmp(*av, "-v") == 0) {
		flags |= HASH_FLAG_VERIFY;
		av++;
		ac--;
	}

	return hash_command("sm3_256", flags, cmdtp, flag, ac, av);
}

#if IS_ENABLED(CONFIG_SM3SUM_VERIFY)
U_BOOT_CMD(sm3sum, 5, 1, do_sm3sum,
	   "compute SM3 message digest",
	   "address count [[*]sum]\n"
	   "  - compute SM3 message digest [save to sum]\n"
	   "sm3sum -v address count [*]sum\n"
	   "  - verify sm3sum of memory area"
);
#else
U_BOOT_CMD(sm3sum, 4, 1, do_sm3sum,
	   "compute SM3 message digest",
	   "address count [[*]sum]\n"
	   "  - compute SM3 message digest [save to sum]"
);
#endif /* IS_ENABLED(CONFIG_SM3SUM_VERIFY) */

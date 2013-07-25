/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <hash.h>
#include <linux/ctype.h>

static int do_hash(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *s;
#ifdef CONFIG_HASH_VERIFY
	int flags = HASH_FLAG_ENV;

	if (argc < 4)
		return CMD_RET_USAGE;
	if (!strcmp(argv[1], "-v")) {
		flags |= HASH_FLAG_VERIFY;
		argc--;
		argv++;
	}
#else
	const int flags = HASH_FLAG_ENV;
#endif
	/* Move forward to 'algorithm' parameter */
	argc--;
	argv++;
	for (s = *argv; *s; s++)
		*s = tolower(*s);
	return hash_command(*argv, flags, cmdtp, flag, argc - 1, argv + 1);
}

#ifdef CONFIG_HASH_VERIFY
U_BOOT_CMD(
	hash,	6,	1,	do_hash,
	"compute hash message digest",
	"algorithm address count [[*]sum_dest]\n"
		"    - compute message digest [save to env var / *address]\n"
	"hash -v algorithm address count [*]sum\n"
		"    - verify hash of memory area with env var / *address"
);
#else
U_BOOT_CMD(
	hash,	5,	1,	do_hash,
	"compute message digest",
	"algorithm address count [[*]sum_dest]\n"
		"    - compute message digest [save to env var / *address]"
);
#endif

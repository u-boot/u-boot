// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <command.h>
#include <env.h>
#include <image.h>
#include <hash.h>
#include <mapmem.h>
#include <u-boot/md5.h>
#include <asm/io.h>

static int do_md5sum(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	int flags = HASH_FLAG_ENV;
	int ac;
	char *const *av;

	if (argc < 3)
		return CMD_RET_USAGE;

	av = argv + 1;
	ac = argc - 1;
	if (IS_ENABLED(CONFIG_MD5SUM_VERIFY) && strcmp(*av, "-v") == 0) {
		flags |= HASH_FLAG_VERIFY;
		av++;
		ac--;
	}

	return hash_command("md5", flags, cmdtp, flag, ac, av);
}

#if IS_ENABLED(CONFIG_MD5SUM_VERIFY)
U_BOOT_CMD(
	md5sum,	5,	1,	do_md5sum,
	"compute MD5 message digest",
	"address count [[*]sum]\n"
		"    - compute MD5 message digest [save to sum]\n"
	"md5sum -v address count [*]sum\n"
		"    - verify md5sum of memory area"
);
#else
U_BOOT_CMD(
	md5sum,	4,	1,	do_md5sum,
	"compute MD5 message digest",
	"address count [[*]sum]\n"
		"    - compute MD5 message digest [save to sum]"
);
#endif /* IS_ENABLED(CONFIG_MD5SUM_VERIFY) */

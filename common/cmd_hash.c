/*
 * Copyright (c) 2012 The Chromium OS Authors.
 *
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
#include <hash.h>

static int do_hash(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
#ifdef CONFIG_HASH_VERIFY
	int verify = 0;

	if (!strcmp(argv[1], "-v")) {
		verify = 1;
		argc--;
		argv++;
	}
#endif
	/* Move forward to 'algorithm' parameter */
	argc--;
	argv++;
	return hash_command(*argv, verify, cmdtp, flag, argc - 1, argv + 1);
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

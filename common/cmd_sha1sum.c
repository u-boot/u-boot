/*
 * (C) Copyright 2011
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 *
 * (C) Copyright 2000
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

#include <common.h>
#include <command.h>
#include <hash.h>
#include <sha1.h>

int do_sha1sum(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int verify = 0;
	int ac;
	char * const *av;

	if (argc < 3)
		return CMD_RET_USAGE;

	av = argv + 1;
	ac = argc - 1;
#ifdef CONFIG_SHA1SUM_VERIFY
	if (strcmp(*av, "-v") == 0) {
		verify = 1;
		av++;
		ac--;
	}
#endif

	return hash_command("sha1", verify, cmdtp, flag, ac, av);
}

#ifdef CONFIG_SHA1SUM_VERIFY
U_BOOT_CMD(
	sha1sum,	5,	1,	do_sha1sum,
	"compute SHA1 message digest",
	"address count [[*]sum]\n"
		"    - compute SHA1 message digest [save to sum]\n"
	"sha1sum -v address count [*]sum\n"
		"    - verify sha1sum of memory area"
);
#else
U_BOOT_CMD(
	sha1sum,	4,	1,	do_sha1sum,
	"compute SHA1 message digest",
	"address count [[*]sum]\n"
		"    - compute SHA1 message digest [save to sum]"
);
#endif

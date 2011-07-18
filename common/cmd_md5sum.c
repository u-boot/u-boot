/*
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
#include <u-boot/md5.h>

static int do_md5sum(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr, len;
	unsigned int i;
	u8 output[16];

	if (argc < 3)
		return cmd_usage(cmdtp);

	addr = simple_strtoul(argv[1], NULL, 16);
	len = simple_strtoul(argv[2], NULL, 16);

	md5_wd((unsigned char *) addr, len, output, CHUNKSZ_MD5);
	printf("md5 for %08lx ... %08lx ==> ", addr, addr + len - 1);
	for (i = 0; i < 16; i++)
		printf("%02x", output[i]);
	printf("\n");

	return 0;
}

U_BOOT_CMD(
	md5sum,	3,	1,	do_md5sum,
	"compute MD5 message digest",
	"address count"
);

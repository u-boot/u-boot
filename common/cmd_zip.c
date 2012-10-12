/*
 * (C) Copyright 2012
 * Lei Wen <leiwen@marvell.com>, Marvell Inc.
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

static int do_zip(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long src, dst;
	unsigned long src_len, dst_len = ~0UL;
	char buf[32];

	switch (argc) {
		case 5:
			dst_len = simple_strtoul(argv[4], NULL, 16);
			/* fall through */
		case 4:
			src = simple_strtoul(argv[1], NULL, 16);
			src_len = simple_strtoul(argv[2], NULL, 16);
			dst = simple_strtoul(argv[3], NULL, 16);
			break;
		default:
			return cmd_usage(cmdtp);
	}

	if (gzip((void *) dst, &dst_len, (void *) src, src_len) != 0)
		return 1;

	printf("Compressed size: %ld = 0x%lX\n", dst_len, dst_len);
	sprintf(buf, "%lX", dst_len);
	setenv("filesize", buf);

	return 0;
}

U_BOOT_CMD(
	zip,	5,	1,	do_zip,
	"zip a memory region",
	"srcaddr srcsize dstaddr [dstsize]"
);

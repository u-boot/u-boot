// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2012
 * Lei Wen <leiwen@marvell.com>, Marvell Inc.
 */

#include <command.h>
#include <env.h>
#include <gzip.h>
#include <mapmem.h>
#include <vsprintf.h>

static int do_zip(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	unsigned long src, dst;
	unsigned long src_len, dst_len = ~0UL;
	void *srcp, *dstp;
	int ret;

	switch (argc) {
		case 5:
			dst_len = hextoul(argv[4], NULL);
			/* fall through */
		case 4:
			src = hextoul(argv[1], NULL);
			src_len = hextoul(argv[2], NULL);
			dst = hextoul(argv[3], NULL);
			break;
		default:
			return cmd_usage(cmdtp);
	}

	srcp = map_sysmem(src, src_len);
	dstp = map_sysmem(dst, dst_len);

	ret = gzip(dstp, &dst_len, srcp, src_len);

	unmap_sysmem(dstp);
	unmap_sysmem(srcp);

	if (ret)
		return CMD_RET_FAILURE;

	printf("Compressed size: %lu = 0x%lX\n", dst_len, dst_len);
	env_set_hex("filesize", dst_len);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	zip,	5,	1,	do_zip,
	"zip a memory region",
	"srcaddr srcsize dstaddr [dstsize]"
);

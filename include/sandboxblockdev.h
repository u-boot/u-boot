/*
 * Copyright (c) 2013, Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SANDBOX_BLOCK_DEV__
#define __SANDBOX_BLOCK_DEV__

struct host_block_dev {
	block_dev_desc_t blk_dev;
	char *filename;
	int fd;
};

int host_dev_bind(int dev, char *filename);

#endif

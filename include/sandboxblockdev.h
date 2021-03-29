/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2013, Henrik Nordstrom <henrik@henriknordstrom.net>
 */

#ifndef __SANDBOX_BLOCK_DEV__
#define __SANDBOX_BLOCK_DEV__

struct host_block_dev {
#ifndef CONFIG_BLK
	struct blk_desc blk_dev;
#endif
	char *filename;
	int fd;
};

/**
 * host_dev_bind() - Bind or unbind a device
 *
 * @dev: Device number (0=first slot)
 * @filename: Host filename to use, or NULL to unbind
 * @removable: true if the block device should mark itself as removable
 */
int host_dev_bind(int dev, char *filename, bool removable);

#endif

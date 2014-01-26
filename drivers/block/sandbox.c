/*
 * Copyright (C) 2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <common.h>
#include <part.h>
#include <os.h>
#include <malloc.h>
#include <sandboxblockdev.h>
#include <asm/errno.h>

static struct host_block_dev host_devices[CONFIG_HOST_MAX_DEVICES];

static struct host_block_dev *find_host_device(int dev)
{
	if (dev >= 0 && dev < CONFIG_HOST_MAX_DEVICES)
		return &host_devices[dev];

	return NULL;
}

static unsigned long host_block_read(int dev, unsigned long start,
				     lbaint_t blkcnt, void *buffer)
{
	struct host_block_dev *host_dev = find_host_device(dev);

	if (!host_dev)
		return -1;
	if (os_lseek(host_dev->fd,
		     start * host_dev->blk_dev.blksz,
		     OS_SEEK_SET) == -1) {
		printf("ERROR: Invalid position\n");
		return -1;
	}
	ssize_t len = os_read(host_dev->fd, buffer,
			      blkcnt * host_dev->blk_dev.blksz);
	if (len >= 0)
		return len / host_dev->blk_dev.blksz;
	return -1;
}

static unsigned long host_block_write(int dev, unsigned long start,
				      lbaint_t blkcnt, const void *buffer)
{
	struct host_block_dev *host_dev = find_host_device(dev);
	if (os_lseek(host_dev->fd,
		     start * host_dev->blk_dev.blksz,
		     OS_SEEK_SET) == -1) {
		printf("ERROR: Invalid position\n");
		return -1;
	}
	ssize_t len = os_write(host_dev->fd, buffer, blkcnt *
			       host_dev->blk_dev.blksz);
	if (len >= 0)
		return len / host_dev->blk_dev.blksz;
	return -1;
}

int host_dev_bind(int dev, char *filename)
{
	struct host_block_dev *host_dev = find_host_device(dev);

	if (!host_dev)
		return -1;
	if (host_dev->blk_dev.priv) {
		os_close(host_dev->fd);
		host_dev->blk_dev.priv = NULL;
	}
	if (host_dev->filename)
		free(host_dev->filename);
	if (filename && *filename) {
		host_dev->filename = strdup(filename);
	} else {
		host_dev->filename = NULL;
		return 0;
	}

	host_dev->fd = os_open(host_dev->filename, OS_O_RDWR);
	if (host_dev->fd == -1) {
		printf("Failed to access host backing file '%s'\n",
		       host_dev->filename);
		return 1;
	}

	block_dev_desc_t *blk_dev = &host_dev->blk_dev;
	blk_dev->if_type = IF_TYPE_HOST;
	blk_dev->priv = host_dev;
	blk_dev->blksz = 512;
	blk_dev->lba = os_lseek(host_dev->fd, 0, OS_SEEK_END) / blk_dev->blksz;
	blk_dev->block_read = host_block_read;
	blk_dev->block_write = host_block_write;
	blk_dev->dev = dev;
	blk_dev->part_type = PART_TYPE_UNKNOWN;
	init_part(blk_dev);

	return 0;
}

int host_get_dev_err(int dev, block_dev_desc_t **blk_devp)
{
	struct host_block_dev *host_dev = find_host_device(dev);

	if (!host_dev)
		return -ENODEV;

	if (!host_dev->blk_dev.priv)
		return -ENOENT;

	*blk_devp = &host_dev->blk_dev;
	return 0;
}

block_dev_desc_t *host_get_dev(int dev)
{
	block_dev_desc_t *blk_dev;

	if (host_get_dev_err(dev, &blk_dev))
		return NULL;

	return blk_dev;
}

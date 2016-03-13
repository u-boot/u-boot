/*
 * Copyright (C) 2013 Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <fdtdec.h>
#include <part.h>
#include <os.h>
#include <malloc.h>
#include <sandboxblockdev.h>
#include <asm/errno.h>
#include <dm/device-internal.h>

DECLARE_GLOBAL_DATA_PTR;

static unsigned long host_block_read(struct udevice *dev,
				     unsigned long start, lbaint_t blkcnt,
				     void *buffer)
{
	struct host_block_dev *host_dev = dev_get_priv(dev);
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);

	if (os_lseek(host_dev->fd, start * block_dev->blksz, OS_SEEK_SET) ==
			-1) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}
	ssize_t len = os_read(host_dev->fd, buffer, blkcnt * block_dev->blksz);
	if (len >= 0)
		return len / block_dev->blksz;
	return -1;
}

static unsigned long host_block_write(struct udevice *dev,
				      unsigned long start, lbaint_t blkcnt,
				      const void *buffer)
{
	struct host_block_dev *host_dev = dev_get_priv(dev);
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);

	if (os_lseek(host_dev->fd, start * block_dev->blksz, OS_SEEK_SET) ==
			-1) {
		printf("ERROR: Invalid block %lx\n", start);
		return -1;
	}
	ssize_t len = os_write(host_dev->fd, buffer, blkcnt * block_dev->blksz);
	if (len >= 0)
		return len / block_dev->blksz;
	return -1;
}

int host_dev_bind(int devnum, char *filename)
{
	struct host_block_dev *host_dev;
	struct udevice *dev;
	char dev_name[20], *str, *fname;
	int ret, fd;

	/* Remove and unbind the old device, if any */
	ret = blk_get_device(IF_TYPE_HOST, devnum, &dev);
	if (ret == 0) {
		ret = device_remove(dev);
		if (ret)
			return ret;
		ret = device_unbind(dev);
		if (ret)
			return ret;
	} else if (ret != -ENODEV) {
		return ret;
	}

	if (!filename)
		return 0;

	snprintf(dev_name, sizeof(dev_name), "host%d", devnum);
	str = strdup(dev_name);
	if (!str)
		return -ENOMEM;
	fname = strdup(filename);
	if (!fname) {
		free(str);
		return -ENOMEM;
	}

	fd = os_open(filename, OS_O_RDWR);
	if (fd == -1) {
		printf("Failed to access host backing file '%s'\n", filename);
		ret = -ENOENT;
		goto err;
	}
	ret = blk_create_device(gd->dm_root, "sandbox_host_blk", str,
				IF_TYPE_HOST, devnum, 512,
				os_lseek(fd, 0, OS_SEEK_END), &dev);
	if (ret)
		goto err_file;
	ret = device_probe(dev);
	if (ret) {
		device_unbind(dev);
		goto err_file;
	}

	host_dev = dev_get_priv(dev);
	host_dev->fd = fd;
	host_dev->filename = fname;

	return blk_prepare_device(dev);
err_file:
	os_close(fd);
err:
	free(fname);
	free(str);
	return ret;
}

int host_get_dev_err(int devnum, struct blk_desc **blk_devp)
{
	struct udevice *dev;
	int ret;

	ret = blk_get_device(IF_TYPE_HOST, devnum, &dev);
	if (ret)
		return ret;
	*blk_devp = dev_get_uclass_platdata(dev);

	return 0;
}

struct blk_desc *host_get_dev(int dev)
{
	struct blk_desc *blk_dev;

	if (host_get_dev_err(dev, &blk_dev))
		return NULL;

	return blk_dev;
}

static const struct blk_ops sandbox_host_blk_ops = {
	.read	= host_block_read,
	.write	= host_block_write,
};

U_BOOT_DRIVER(sandbox_host_blk) = {
	.name		= "sandbox_host_blk",
	.id		= UCLASS_BLK,
	.ops		= &sandbox_host_blk_ops,
	.priv_auto_alloc_size	= sizeof(struct host_block_dev),
};

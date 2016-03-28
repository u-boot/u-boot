/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

int blk_first_device(int if_type, struct udevice **devp)
{
	struct blk_desc *desc;
	int ret;

	ret = uclass_first_device(UCLASS_BLK, devp);
	if (ret)
		return ret;
	if (!*devp)
		return -ENODEV;
	do {
		desc = dev_get_uclass_platdata(*devp);
		if (desc->if_type == if_type)
			return 0;
		ret = uclass_next_device(devp);
		if (ret)
			return ret;
	} while (*devp);

	return -ENODEV;
}

int blk_next_device(struct udevice **devp)
{
	struct blk_desc *desc;
	int ret, if_type;

	desc = dev_get_uclass_platdata(*devp);
	if_type = desc->if_type;
	do {
		ret = uclass_next_device(devp);
		if (ret)
			return ret;
		if (!*devp)
			return -ENODEV;
		desc = dev_get_uclass_platdata(*devp);
		if (desc->if_type == if_type)
			return 0;
	} while (1);
}

int blk_get_device(int if_type, int devnum, struct udevice **devp)
{
	struct uclass *uc;
	struct udevice *dev;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev(dev, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		debug("%s: if_type=%d, devnum=%d: %s, %d, %d\n", __func__,
		      if_type, devnum, dev->name, desc->if_type, desc->devnum);
		if (desc->if_type == if_type && desc->devnum == devnum) {
			*devp = dev;
			return device_probe(dev);
		}
	}

	return -ENODEV;
}

unsigned long blk_dread(struct blk_desc *block_dev, lbaint_t start,
			lbaint_t blkcnt, void *buffer)
{
	struct udevice *dev = block_dev->bdev;
	const struct blk_ops *ops = blk_get_ops(dev);
	ulong blks_read;

	if (!ops->read)
		return -ENOSYS;

	if (blkcache_read(block_dev->if_type, block_dev->devnum,
			  start, blkcnt, block_dev->blksz, buffer))
		return blkcnt;
	blks_read = ops->read(dev, start, blkcnt, buffer);
	if (blks_read == blkcnt)
		blkcache_fill(block_dev->if_type, block_dev->devnum,
			      start, blkcnt, block_dev->blksz, buffer);

	return blks_read;
}

unsigned long blk_dwrite(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt, const void *buffer)
{
	struct udevice *dev = block_dev->bdev;
	const struct blk_ops *ops = blk_get_ops(dev);

	if (!ops->write)
		return -ENOSYS;

	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return ops->write(dev, start, blkcnt, buffer);
}

unsigned long blk_derase(struct blk_desc *block_dev, lbaint_t start,
			 lbaint_t blkcnt)
{
	struct udevice *dev = block_dev->bdev;
	const struct blk_ops *ops = blk_get_ops(dev);

	if (!ops->erase)
		return -ENOSYS;

	blkcache_invalidate(block_dev->if_type, block_dev->devnum);
	return ops->erase(dev, start, blkcnt);
}

int blk_prepare_device(struct udevice *dev)
{
	struct blk_desc *desc = dev_get_uclass_platdata(dev);

	part_init(desc);

	return 0;
}

int blk_create_device(struct udevice *parent, const char *drv_name,
		      const char *name, int if_type, int devnum, int blksz,
		      lbaint_t size, struct udevice **devp)
{
	struct blk_desc *desc;
	struct udevice *dev;
	int ret;

	ret = device_bind_driver(parent, drv_name, name, &dev);
	if (ret)
		return ret;
	desc = dev_get_uclass_platdata(dev);
	desc->if_type = if_type;
	desc->blksz = blksz;
	desc->lba = size / blksz;
	desc->part_type = PART_TYPE_UNKNOWN;
	desc->bdev = dev;
	desc->devnum = devnum;
	*devp = dev;

	return 0;
}

int blk_unbind_all(int if_type)
{
	struct uclass *uc;
	struct udevice *dev, *next;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;
	uclass_foreach_dev_safe(dev, next, uc) {
		struct blk_desc *desc = dev_get_uclass_platdata(dev);

		if (desc->if_type == if_type) {
			ret = device_remove(dev);
			if (ret)
				return ret;
			ret = device_unbind(dev);
			if (ret)
				return ret;
		}
	}

	return 0;
}

UCLASS_DRIVER(blk) = {
	.id		= UCLASS_BLK,
	.name		= "blk",
	.per_device_platdata_auto_alloc_size = sizeof(struct blk_desc),
};

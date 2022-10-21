// SPDX-License-Identifier: GPL-2.0+
/*
 *  Software partition device (UCLASS_PARTITION)
 *
 *  Copyright (c) 2021 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#define LOG_CATEGORY UCLASS_PARTITION

#include <common.h>
#include <blk.h>
#include <dm.h>
#include <log.h>
#include <part.h>
#include <vsprintf.h>
#include <dm/device-internal.h>
#include <dm/lists.h>

int part_create_block_devices(struct udevice *blk_dev)
{
	int part, count;
	struct blk_desc *desc = dev_get_uclass_plat(blk_dev);
	struct disk_partition info;
	struct disk_part *part_data;
	char devname[32];
	struct udevice *dev;
	int ret;

	if (!CONFIG_IS_ENABLED(PARTITIONS) || !blk_enabled())
		return 0;

	if (device_get_uclass_id(blk_dev) != UCLASS_BLK)
		return 0;

	/* Add devices for each partition */
	for (count = 0, part = 1; part <= MAX_SEARCH_PARTITIONS; part++) {
		if (part_get_info(desc, part, &info))
			continue;
		snprintf(devname, sizeof(devname), "%s:%d", blk_dev->name,
			 part);

		ret = device_bind_driver(blk_dev, "blk_partition",
					 strdup(devname), &dev);
		if (ret)
			return ret;

		part_data = dev_get_uclass_plat(dev);
		part_data->partnum = part;
		part_data->gpt_part_info = info;
		count++;

		ret = device_probe(dev);
		if (ret) {
			debug("Can't probe\n");
			count--;
			device_unbind(dev);

			continue;
		}
	}
	debug("%s: %d partitions found in %s\n", __func__, count,
	      blk_dev->name);

	return 0;
}

static ulong part_blk_read(struct udevice *dev, lbaint_t start,
			   lbaint_t blkcnt, void *buffer)
{
	struct udevice *parent;
	struct disk_part *part;
	const struct blk_ops *ops;

	parent = dev_get_parent(dev);
	ops = blk_get_ops(parent);
	if (!ops->read)
		return -ENOSYS;

	part = dev_get_uclass_plat(dev);
	if (start >= part->gpt_part_info.size)
		return 0;

	if ((start + blkcnt) > part->gpt_part_info.size)
		blkcnt = part->gpt_part_info.size - start;
	start += part->gpt_part_info.start;

	return ops->read(parent, start, blkcnt, buffer);
}

static ulong part_blk_write(struct udevice *dev, lbaint_t start,
			    lbaint_t blkcnt, const void *buffer)
{
	struct udevice *parent;
	struct disk_part *part;
	const struct blk_ops *ops;

	parent = dev_get_parent(dev);
	ops = blk_get_ops(parent);
	if (!ops->write)
		return -ENOSYS;

	part = dev_get_uclass_plat(dev);
	if (start >= part->gpt_part_info.size)
		return 0;

	if ((start + blkcnt) > part->gpt_part_info.size)
		blkcnt = part->gpt_part_info.size - start;
	start += part->gpt_part_info.start;

	return ops->write(parent, start, blkcnt, buffer);
}

static ulong part_blk_erase(struct udevice *dev, lbaint_t start,
			    lbaint_t blkcnt)
{
	struct udevice *parent;
	struct disk_part *part;
	const struct blk_ops *ops;

	parent = dev_get_parent(dev);
	ops = blk_get_ops(parent);
	if (!ops->erase)
		return -ENOSYS;

	part = dev_get_uclass_plat(dev);
	if (start >= part->gpt_part_info.size)
		return 0;

	if ((start + blkcnt) > part->gpt_part_info.size)
		blkcnt = part->gpt_part_info.size - start;
	start += part->gpt_part_info.start;

	return ops->erase(parent, start, blkcnt);
}

static const struct blk_ops blk_part_ops = {
	.read	= part_blk_read,
	.write	= part_blk_write,
	.erase	= part_blk_erase,
};

U_BOOT_DRIVER(blk_partition) = {
	.name		= "blk_partition",
	.id		= UCLASS_PARTITION,
	.ops		= &blk_part_ops,
};

/*
 * BLOCK IO APIs
 */
static struct blk_desc *dev_get_blk(struct udevice *dev)
{
	struct blk_desc *desc;

	switch (device_get_uclass_id(dev)) {
	/*
	 * We won't support UCLASS_BLK with dev_* interfaces.
	 */
	case UCLASS_PARTITION:
		desc = dev_get_uclass_plat(dev_get_parent(dev));
		break;
	default:
		desc = NULL;
		break;
	}

	return desc;
}

unsigned long disk_blk_read(struct udevice *dev, lbaint_t start,
			    lbaint_t blkcnt, void *buffer)
{
	struct blk_desc *desc;
	const struct blk_ops *ops;
	struct disk_part *part;
	lbaint_t start_in_disk;
	ulong blks_read;

	desc = dev_get_blk(dev);
	if (!desc)
		return -ENOSYS;

	ops = blk_get_ops(dev);
	if (!ops->read)
		return -ENOSYS;

	start_in_disk = start;
	if (device_get_uclass_id(dev) == UCLASS_PARTITION) {
		part = dev_get_uclass_plat(dev);
		start_in_disk += part->gpt_part_info.start;
	}

	if (blkcache_read(desc->uclass_id, desc->devnum, start_in_disk, blkcnt,
			  desc->blksz, buffer))
		return blkcnt;
	blks_read = ops->read(dev, start, blkcnt, buffer);
	if (blks_read == blkcnt)
		blkcache_fill(desc->uclass_id, desc->devnum, start_in_disk,
			      blkcnt, desc->blksz, buffer);

	return blks_read;
}

unsigned long disk_blk_write(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *desc;
	const struct blk_ops *ops;

	desc = dev_get_blk(dev);
	if (!desc)
		return -ENOSYS;

	ops = blk_get_ops(dev);
	if (!ops->write)
		return -ENOSYS;

	blkcache_invalidate(desc->uclass_id, desc->devnum);

	return ops->write(dev, start, blkcnt, buffer);
}

unsigned long disk_blk_erase(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt)
{
	struct blk_desc *desc;
	const struct blk_ops *ops;

	desc = dev_get_blk(dev);
	if (!desc)
		return -ENOSYS;

	ops = blk_get_ops(dev);
	if (!ops->erase)
		return -ENOSYS;

	blkcache_invalidate(desc->uclass_id, desc->devnum);

	return ops->erase(dev, start, blkcnt);
}

UCLASS_DRIVER(partition) = {
	.id		= UCLASS_PARTITION,
	.per_device_plat_auto	= sizeof(struct disk_part),
	.name		= "partition",
};

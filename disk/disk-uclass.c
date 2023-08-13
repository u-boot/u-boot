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
/**
 * disk_blk_read() - Read from a block device partition
 *
 * @dev: Device to read from (partition udevice)
 * @start: Start block for the read (from start of partition)
 * @blkcnt: Number of blocks to read (within the partition)
 * @buffer: Place to put the data
 * @return number of blocks read (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
unsigned long disk_blk_read(struct udevice *dev, lbaint_t start,
			    lbaint_t blkcnt, void *buffer)
{
	struct disk_part *part = dev_get_uclass_plat(dev);

	if (device_get_uclass_id(dev) != UCLASS_PARTITION)
		return -ENOSYS;

	return blk_read(dev_get_parent(dev), start + part->gpt_part_info.start,
			blkcnt, buffer);
}

/**
 * disk_blk_write() - Write to a block device
 *
 * @dev: Device to write to
 * @start: Start block for the write
 * @blkcnt: Number of blocks to write
 * @buffer: Data to write
 * @return number of blocks written (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
unsigned long disk_blk_write(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt, const void *buffer)
{
	if (device_get_uclass_id(dev) != UCLASS_PARTITION)
		return -ENOSYS;

	return blk_write(dev_get_parent(dev), start, blkcnt, buffer);
}

/**
 * disk_blk_erase() - Erase part of a block device
 *
 * @dev: Device to erase
 * @start: Start block for the erase
 * @blkcnt: Number of blocks to erase
 * @return number of blocks erased (which may be less than @blkcnt),
 * or -ve on error. This never returns 0 unless @blkcnt is 0
 */
unsigned long disk_blk_erase(struct udevice *dev, lbaint_t start,
			     lbaint_t blkcnt)
{
	if (device_get_uclass_id(dev) != UCLASS_PARTITION)
		return -ENOSYS;

	return blk_erase(dev_get_parent(dev), start, blkcnt);
}

UCLASS_DRIVER(partition) = {
	.id		= UCLASS_PARTITION,
	.per_device_plat_auto	= sizeof(struct disk_part),
	.name		= "partition",
};

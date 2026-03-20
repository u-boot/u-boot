// SPDX-License-Identifier: GPL-2.0+
/*
 * Block device backend for imagemap (UCLASS_IMAGEMAP driver)
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#include <blk.h>
#include <dm.h>
#include <imagemap.h>
#include <memalign.h>
#include <part.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <log.h>

/**
 * struct imagemap_blk_plat - Platform data set before probe
 *
 * @part_start:	Partition start LBA
 * @part_size:	Partition size in blocks
 */
struct imagemap_blk_plat {
	lbaint_t part_start;
	lbaint_t part_size;
};

/**
 * struct imagemap_blk_priv - Runtime private data
 *
 * @part_start:	Partition start LBA
 * @part_size:	Partition size in blocks
 */
struct imagemap_blk_priv {
	lbaint_t part_start;
	lbaint_t part_size;
};

/**
 * blk_read_partial() - Read a partial sector via bounce buffer
 *
 * Reads one full sector into a stack-allocated bounce buffer, then
 * copies @len bytes starting at byte offset @skip within that sector
 * into @dst.
 *
 * @desc:	Block device descriptor
 * @lba:	Absolute LBA of the sector to read
 * @offset:	Byte offset within the sector
 * @len:	Number of bytes to copy
 * @dst:	Destination buffer
 * Return: 0 on success, -EIO on read failure
 */
static int blk_read_partial(struct blk_desc *desc, lbaint_t lba,
			    ulong offset, ulong len, void *dst)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, sec, desc->blksz);

	if (blk_dread(desc, lba, 1, sec) != 1)
		return -EIO;

	memcpy(dst, sec + offset, len);

	return 0;
}

static int imagemap_blk_read(struct udevice *dev, loff_t src,
			     ulong size, void *dst)
{
	struct imagemap_blk_priv *priv = dev_get_priv(dev);
	struct blk_desc *desc = dev_get_uclass_plat(dev_get_parent(dev));
	ulong blksz = desc->blksz;
	ulong head = src % blksz;
	lbaint_t lba;
	lbaint_t n;
	u8 *out = dst;
	int ret;

	/* Bounds check — use 64-bit arithmetic to avoid overflow */
	if (src + size > (u64)priv->part_size * blksz) {
		log_err("imagemap_blk: read at 0x%llx+0x%lx exceeds partition size\n",
			(unsigned long long)src, size);
		return -EINVAL;
	}

	lba = priv->part_start + (lbaint_t)(src / blksz);

	/* Handle unaligned head */
	if (head) {
		ulong chunk = min(size, blksz - head);

		ret = blk_read_partial(desc, lba, head, chunk, out);
		if (ret)
			return ret;

		out += chunk;
		size -= chunk;
		lba++;
	}

	/* Aligned middle — read whole sectors directly into dst */
	if (size >= blksz) {
		n = size / blksz;

		if (blk_dread(desc, lba, n, out) != n)
			return -EIO;

		out += n * blksz;
		size -= n * blksz;
		lba += n;
	}

	/* Handle unaligned tail */
	if (size) {
		ret = blk_read_partial(desc, lba, 0, size, out);
		if (ret)
			return ret;
	}

	return 0;
}

static int imagemap_blk_probe(struct udevice *dev)
{
	struct imagemap_blk_plat *plat = dev_get_plat(dev);
	struct imagemap_blk_priv *priv = dev_get_priv(dev);

	priv->part_start = plat->part_start;
	priv->part_size = plat->part_size;

	return 0;
}


static const struct imagemap_ops imagemap_blk_ops = {
	.read	= imagemap_blk_read,
};

U_BOOT_DRIVER(imagemap_blk) = {
	.name		= "imagemap_blk",
	.id		= UCLASS_IMAGEMAP,
	.ops		= &imagemap_blk_ops,
	.probe		= imagemap_blk_probe,
	.plat_auto	= sizeof(struct imagemap_blk_plat),
	.priv_auto	= sizeof(struct imagemap_blk_priv),
};

static int imagemap_blk_dev_create(struct udevice *dev, const char *name,
				   int part, struct udevice **devp)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	struct disk_partition info;
	struct imagemap_blk_plat *plat;
	struct udevice *imdev;
	int ret;

	ret = part_get_info_by_name(desc, name, &info);
	if (ret < 0)
		return ret;

	ret = device_bind_driver(desc->bdev, "imagemap_blk",
				 name, &imdev);
	if (ret)
		return ret;

	plat = dev_get_plat(imdev);
	plat->part_start = info.start;
	plat->part_size = info.size;

	ret = device_probe(imdev);
	if (ret) {
		device_unbind(imdev);
		return ret;
	}

	*devp = imdev;

	return 0;
}

IMAGEMAP_BACKEND(imagemap_blk) = {
	.uclass	= UCLASS_BLK,
	.create	= imagemap_blk_dev_create,
};

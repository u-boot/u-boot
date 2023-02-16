// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Addiva Elektronik
 * Author: Tobias Waldekranz <tobias@waldekranz.com>
 */

#include <common.h>
#include <blk.h>
#include <blkmap.h>
#include <dm.h>
#include <malloc.h>
#include <mapmem.h>
#include <part.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/root.h>

struct blkmap;

/**
 * struct blkmap_slice - Region mapped to a blkmap
 *
 * Common data for a region mapped to a blkmap, specialized by each
 * map type.
 *
 * @node: List node used to associate this slice with a blkmap
 * @blknr: Start block number of the mapping
 * @blkcnt: Number of blocks covered by this mapping
 */
struct blkmap_slice {
	struct list_head node;

	lbaint_t blknr;
	lbaint_t blkcnt;

	/**
	 * @read: - Read from slice
	 *
	 * @read.bm: Blkmap to which this slice belongs
	 * @read.bms: This slice
	 * @read.blknr: Start block number to read from
	 * @read.blkcnt: Number of blocks to read
	 * @read.buffer: Buffer to store read data to
	 */
	ulong (*read)(struct blkmap *bm, struct blkmap_slice *bms,
		      lbaint_t blknr, lbaint_t blkcnt, void *buffer);

	/**
	 * @write: - Write to slice
	 *
	 * @write.bm: Blkmap to which this slice belongs
	 * @write.bms: This slice
	 * @write.blknr: Start block number to write to
	 * @write.blkcnt: Number of blocks to write
	 * @write.buffer: Data to be written
	 */
	ulong (*write)(struct blkmap *bm, struct blkmap_slice *bms,
		       lbaint_t blknr, lbaint_t blkcnt, const void *buffer);

	/**
	 * @destroy: - Tear down slice
	 *
	 * @read.bm: Blkmap to which this slice belongs
	 * @read.bms: This slice
	 */
	void (*destroy)(struct blkmap *bm, struct blkmap_slice *bms);
};

/**
 * struct blkmap - Block map
 *
 * Data associated with a blkmap.
 *
 * @label: Human readable name of this blkmap
 * @blk: Underlying block device
 * @slices: List of slices associated with this blkmap
 */
struct blkmap {
	char *label;
	struct udevice *blk;
	struct list_head slices;
};

static bool blkmap_slice_contains(struct blkmap_slice *bms, lbaint_t blknr)
{
	return (blknr >= bms->blknr) && (blknr < (bms->blknr + bms->blkcnt));
}

static bool blkmap_slice_available(struct blkmap *bm, struct blkmap_slice *new)
{
	struct blkmap_slice *bms;
	lbaint_t first, last;

	first = new->blknr;
	last = new->blknr + new->blkcnt - 1;

	list_for_each_entry(bms, &bm->slices, node) {
		if (blkmap_slice_contains(bms, first) ||
		    blkmap_slice_contains(bms, last) ||
		    blkmap_slice_contains(new, bms->blknr) ||
		    blkmap_slice_contains(new, bms->blknr + bms->blkcnt - 1))
			return false;
	}

	return true;
}

static int blkmap_slice_add(struct blkmap *bm, struct blkmap_slice *new)
{
	struct blk_desc *bd = dev_get_uclass_plat(bm->blk);
	struct list_head *insert = &bm->slices;
	struct blkmap_slice *bms;

	if (!blkmap_slice_available(bm, new))
		return -EBUSY;

	list_for_each_entry(bms, &bm->slices, node) {
		if (bms->blknr < new->blknr)
			continue;

		insert = &bms->node;
		break;
	}

	list_add_tail(&new->node, insert);

	/* Disk might have grown, update the size */
	bms = list_last_entry(&bm->slices, struct blkmap_slice, node);
	bd->lba = bms->blknr + bms->blkcnt;
	return 0;
}

static ulong blkmap_blk_read_slice(struct blkmap *bm, struct blkmap_slice *bms,
				   lbaint_t blknr, lbaint_t blkcnt,
				   void *buffer)
{
	lbaint_t nr, cnt;

	nr = blknr - bms->blknr;
	cnt = (blkcnt < bms->blkcnt) ? blkcnt : bms->blkcnt;
	return bms->read(bm, bms, nr, cnt, buffer);
}

static ulong blkmap_blk_read(struct udevice *dev, lbaint_t blknr,
			     lbaint_t blkcnt, void *buffer)
{
	struct blk_desc *bd = dev_get_uclass_plat(dev);
	struct blkmap *bm = dev_get_plat(dev->parent);
	struct blkmap_slice *bms;
	lbaint_t cnt, total = 0;

	list_for_each_entry(bms, &bm->slices, node) {
		if (!blkmap_slice_contains(bms, blknr))
			continue;

		cnt = blkmap_blk_read_slice(bm, bms, blknr, blkcnt, buffer);
		blknr += cnt;
		blkcnt -= cnt;
		buffer += cnt << bd->log2blksz;
		total += cnt;
	}

	return total;
}

static ulong blkmap_blk_write_slice(struct blkmap *bm, struct blkmap_slice *bms,
				    lbaint_t blknr, lbaint_t blkcnt,
				    const void *buffer)
{
	lbaint_t nr, cnt;

	nr = blknr - bms->blknr;
	cnt = (blkcnt < bms->blkcnt) ? blkcnt : bms->blkcnt;
	return bms->write(bm, bms, nr, cnt, buffer);
}

static ulong blkmap_blk_write(struct udevice *dev, lbaint_t blknr,
			      lbaint_t blkcnt, const void *buffer)
{
	struct blk_desc *bd = dev_get_uclass_plat(dev);
	struct blkmap *bm = dev_get_plat(dev->parent);
	struct blkmap_slice *bms;
	lbaint_t cnt, total = 0;

	list_for_each_entry(bms, &bm->slices, node) {
		if (!blkmap_slice_contains(bms, blknr))
			continue;

		cnt = blkmap_blk_write_slice(bm, bms, blknr, blkcnt, buffer);
		blknr += cnt;
		blkcnt -= cnt;
		buffer += cnt << bd->log2blksz;
		total += cnt;
	}

	return total;
}

static const struct blk_ops blkmap_blk_ops = {
	.read	= blkmap_blk_read,
	.write	= blkmap_blk_write,
};

U_BOOT_DRIVER(blkmap_blk) = {
	.name		= "blkmap_blk",
	.id		= UCLASS_BLK,
	.ops		= &blkmap_blk_ops,
};

int blkmap_dev_bind(struct udevice *dev)
{
	struct blkmap *bm = dev_get_plat(dev);
	struct blk_desc *bd;
	int err;

	err = blk_create_devicef(dev, "blkmap_blk", "blk", UCLASS_BLKMAP,
				 dev_seq(dev), 512, 0, &bm->blk);
	if (err)
		return log_msg_ret("blk", err);

	INIT_LIST_HEAD(&bm->slices);

	bd = dev_get_uclass_plat(bm->blk);
	snprintf(bd->vendor, BLK_VEN_SIZE, "U-Boot");
	snprintf(bd->product, BLK_PRD_SIZE, "blkmap");
	snprintf(bd->revision, BLK_REV_SIZE, "1.0");

	/* EFI core isn't keen on zero-sized disks, so we lie. This is
	 * updated with the correct size once the user adds a
	 * mapping.
	 */
	bd->lba = 1;

	return 0;
}

int blkmap_dev_unbind(struct udevice *dev)
{
	struct blkmap *bm = dev_get_plat(dev);
	struct blkmap_slice *bms, *tmp;
	int err;

	list_for_each_entry_safe(bms, tmp, &bm->slices, node) {
		list_del(&bms->node);
		free(bms);
	}

	err = device_remove(bm->blk, DM_REMOVE_NORMAL);
	if (err)
		return err;

	return device_unbind(bm->blk);
}

U_BOOT_DRIVER(blkmap_root) = {
	.name		= "blkmap_dev",
	.id		= UCLASS_BLKMAP,
	.bind		= blkmap_dev_bind,
	.unbind		= blkmap_dev_unbind,
	.plat_auto	= sizeof(struct blkmap),
};

struct udevice *blkmap_from_label(const char *label)
{
	struct udevice *dev;
	struct uclass *uc;
	struct blkmap *bm;

	uclass_id_foreach_dev(UCLASS_BLKMAP, dev, uc) {
		bm = dev_get_plat(dev);
		if (bm->label && !strcmp(label, bm->label))
			return dev;
	}

	return NULL;
}

int blkmap_create(const char *label, struct udevice **devp)
{
	char *hname, *hlabel;
	struct udevice *dev;
	struct blkmap *bm;
	size_t namelen;
	int err;

	dev = blkmap_from_label(label);
	if (dev) {
		err = -EBUSY;
		goto err;
	}

	hlabel = strdup(label);
	if (!hlabel) {
		err = -ENOMEM;
		goto err;
	}

	namelen = strlen("blkmap-") + strlen(label) + 1;
	hname = malloc(namelen);
	if (!hname) {
		err = -ENOMEM;
		goto err_free_hlabel;
	}

	strlcpy(hname, "blkmap-", namelen);
	strlcat(hname, label, namelen);

	err = device_bind_driver(dm_root(), "blkmap_dev", hname, &dev);
	if (err)
		goto err_free_hname;

	device_set_name_alloced(dev);
	bm = dev_get_plat(dev);
	bm->label = hlabel;

	if (devp)
		*devp = dev;

	return 0;

err_free_hname:
	free(hname);
err_free_hlabel:
	free(hlabel);
err:
	return err;
}

int blkmap_destroy(struct udevice *dev)
{
	int err;

	err = device_remove(dev, DM_REMOVE_NORMAL);
	if (err)
		return err;

	return device_unbind(dev);
}

UCLASS_DRIVER(blkmap) = {
	.id		= UCLASS_BLKMAP,
	.name		= "blkmap",
};

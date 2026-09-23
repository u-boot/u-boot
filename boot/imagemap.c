// SPDX-License-Identifier: GPL-2.0+
/*
 * On-demand image loading from storage (UCLASS_IMAGEMAP)
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#define LOG_CATEGORY UCLASS_IMAGEMAP

#include <blk.h>
#include <dm.h>
#include <imagemap.h>
#include <lmb.h>
#include <mapmem.h>
#include <memalign.h>
#include <part.h>
#include <spl.h>
#include <asm/cache.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <log.h>

/**
 * struct imagemap_plat - Platform data set before probe
 *
 * @part_start:	Partition start LBA
 * @part_size:	Partition size in blocks
 * @hwpart:	Hardware partition to select before each read
 */
struct imagemap_plat {
	lbaint_t part_start;
	lbaint_t part_size;
	int hwpart;
};

/**
 * struct imagemap_blk - Runtime block-read state (the spl_load_info priv)
 *
 * @desc:	Block device descriptor
 * @part_start:	Partition start LBA
 * @part_size:	Partition size in blocks
 * @hwpart:	Hardware partition to (re-)select before each read.  For a UBI
 *		block device this selects the UBI volume (encoded in
 *		blk_desc->hwpart by the "ubi" partition driver); for normal
 *		media it is the hardware partition (0 for the user area).
 */
struct imagemap_blk {
	struct blk_desc *desc;
	lbaint_t part_start;
	lbaint_t part_size;
	int hwpart;
};

void *imagemap_lookup(struct udevice *dev, loff_t img_offset, ulong size)
{
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);
	const struct imagemap_region *r;

	alist_for_each(r, &priv->regions) {
		/*
		 * Check whether [img_offset, img_offset + size) is fully
		 * contained within [r->img_offset, r->img_offset + r->size).
		 *
		 * The three conditions are ordered to avoid unsigned
		 * underflow in the subtraction on the third line:
		 *
		 *  1) img_offset >= r->img_offset
		 *     The requested start is at or past the region start.
		 *
		 *  2) img_offset - r->img_offset <= r->size
		 *     The offset into the region does not exceed the
		 *     region size.  This guard is essential: without it,
		 *     the subtraction in (3) wraps to a huge value on
		 *     LP64 where ulong and loff_t have the same rank
		 *     and the arithmetic is performed as unsigned.
		 *
		 *  3) size <= r->size - (img_offset - r->img_offset)
		 *     The requested range fits in the remaining space.
		 *     Safe because (2) guarantees the subtraction does
		 *     not underflow.
		 */
		if (img_offset >= r->img_offset &&
		    img_offset - r->img_offset <= r->size &&
		    size <= r->size - (img_offset - r->img_offset))
			return (char *)r->ram + (img_offset - r->img_offset);
	}

	return NULL;
}

/**
 * imagemap_record() - Record a region in the translation table
 *
 * If an entry with the same img_offset already exists and the new size
 * is larger, update the existing entry. Otherwise add a new entry.
 *
 * @dev:		The imagemap device
 * @img_offset:		Byte offset within the source image
 * @size:		Region size
 * @ram:		RAM pointer where the region was loaded
 * @lmb_reserved:	true if this region was allocated via LMB
 * Return: pointer to the region entry, or NULL if the table is full
 */
static struct imagemap_region *
imagemap_record(struct udevice *dev, loff_t img_offset, ulong size,
		void *ram, bool lmb_reserved)
{
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);
	struct imagemap_region *r;
	struct imagemap_region entry;

	/* Check for an existing entry at the same base that we can extend */
	alist_for_each(r, &priv->regions) {
		if (r->img_offset == img_offset) {
			r->size = size;
			r->ram = ram;
			r->lmb_reserved = lmb_reserved;
			return r;
		}
	}

	/* Append new region */
	entry.img_offset = img_offset;
	entry.size = size;
	entry.ram = ram;
	entry.lmb_reserved = lmb_reserved;

	r = alist_add(&priv->regions, entry);
	if (!r) {
		log_err("imagemap: cannot add region (out of memory)\n");
		return NULL;
	}

	return r;
}

/**
 * imagemap_read_exact() - Read [off, off+size) byte-exact into @dst
 *
 * For byte-addressed sources (bl_len == 1) this is a straight
 * spl_load_region().  For block sources it reads the fully-contained
 * middle blocks straight into @dst (zero-copy) and bounces only the
 * partial head/tail block, so the wanted bytes land exactly at @dst
 * without over-reading the destination buffer.
 *
 * Return: 0 on success, negative errno on failure
 */
static int imagemap_read_exact(struct spl_load_info *info, loff_t off,
			       ulong size, void *dst)
{
	ulong bl_len = spl_get_bl_len(info);
	ulong overhead = off & (bl_len - 1);
	u8 *out = dst;
	loff_t cur = off;
	ulong left = size;

	if (bl_len == 1)
		return spl_load_region(info, off, size, dst) < 0 ? -EIO : 0;

	/* Partial head block: bounce and copy the wanted tail of the block */
	if (overhead) {
		ALLOC_CACHE_ALIGN_BUFFER(u8, blk, bl_len);
		ulong chunk = min(left, bl_len - overhead);

		if (info->read(info, cur - overhead, bl_len, blk) < bl_len)
			return -EIO;
		memcpy(out, blk + overhead, chunk);
		out += chunk;
		cur += chunk;
		left -= chunk;
	}

	/* Aligned middle: whole blocks straight into @dst (zero-copy) */
	if (left >= bl_len) {
		ulong nbytes = left & ~(bl_len - 1);

		if (info->read(info, cur, nbytes, out) < nbytes)
			return -EIO;
		out += nbytes;
		cur += nbytes;
		left -= nbytes;
	}

	/* Partial tail block: bounce and copy the wanted head of the block */
	if (left) {
		ALLOC_CACHE_ALIGN_BUFFER(u8, blk, bl_len);

		if (info->read(info, cur, bl_len, blk) < bl_len)
			return -EIO;
		memcpy(out, blk, left);
	}

	return 0;
}

void *imagemap_map(struct udevice *dev, loff_t img_offset, ulong size)
{
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);
	ulong bl_len = spl_get_bl_len(&priv->info);
	ulong overhead = img_offset & (bl_len - 1);
	loff_t base_off = img_offset - overhead;
	ulong read_size = ALIGN(size + overhead, bl_len);
	phys_addr_t addr;
	phys_size_t alloc_size;
	void *base;
	int ret;
	struct imagemap_region *r;
	void *p;

	/* Return existing mapping if the range is already covered */
	p = imagemap_lookup(dev, img_offset, size);
	if (p)
		return p;

	alloc_size = ALIGN(read_size, ARCH_DMA_MINALIGN);

	/*
	 * Extend a block-aligned region at the same base offset to the
	 * larger size, re-reading the full range from storage.
	 */
	alist_for_each(r, &priv->regions) {
		if (r->img_offset == base_off && r->size < read_size) {
			addr = map_to_sysmem(r->ram);

			/* Free old LMB reservation if we own it */
			if (r->lmb_reserved)
				lmb_free(addr,
					 ALIGN(r->size, ARCH_DMA_MINALIGN),
					 LMB_NONE);

			/* Try to re-reserve at the same address with new size */
			if (lmb_alloc_mem(LMB_MEM_ALLOC_ADDR, 0, &addr,
					  alloc_size, LMB_NONE)) {
				/* In-place extend failed, allocate elsewhere */
				if (lmb_alloc_mem(LMB_MEM_ALLOC_ANY,
						  ARCH_DMA_MINALIGN,
						  &addr, alloc_size,
						  LMB_NONE)) {
					log_err("imagemap: LMB alloc failed (0x%lx bytes)\n",
						(ulong)alloc_size);
					return ERR_PTR(-ENOMEM);
				}
			}
			base = map_sysmem(addr, alloc_size);

			ret = spl_load_region(&priv->info, img_offset, size,
					      base);
			if (ret < 0) {
				log_err("imagemap: read failed at offset 0x%llx (size 0x%lx): %d\n",
					(unsigned long long)img_offset, size, ret);
				lmb_free(addr, alloc_size, LMB_NONE);
				return ERR_PTR(ret);
			}
			r->size = read_size;
			r->ram = base;
			r->lmb_reserved = true;

			return base + overhead;
		}
	}

	/* New region - allocate from LMB */
	if (lmb_alloc_mem(LMB_MEM_ALLOC_ANY, ARCH_DMA_MINALIGN,
			  &addr, alloc_size, LMB_NONE)) {
		log_err("imagemap: LMB alloc failed (0x%lx bytes)\n",
			(ulong)alloc_size);
		return ERR_PTR(-ENOMEM);
	}

	base = map_sysmem(addr, alloc_size);

	ret = spl_load_region(&priv->info, img_offset, size, base);
	if (ret < 0) {
		log_err("imagemap: read failed at offset 0x%llx (size 0x%lx): %d\n",
			(unsigned long long)img_offset, size, ret);
		lmb_free(addr, alloc_size, LMB_NONE);
		return ERR_PTR(ret);
	}

	if (!imagemap_record(dev, base_off, read_size, base, true)) {
		lmb_free(addr, alloc_size, LMB_NONE);
		return ERR_PTR(-ENOMEM);
	}

	return base + overhead;
}

void *imagemap_map_to(struct udevice *dev, loff_t img_offset,
		      ulong size, void *dst)
{
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);
	void *p;
	int ret;

	/* If already mapped to this exact destination, return it */
	p = imagemap_lookup(dev, img_offset, size);
	if (p && p == dst)
		return p;

	ret = imagemap_read_exact(&priv->info, img_offset, size, dst);
	if (ret) {
		log_err("imagemap: read failed at offset 0x%llx (size 0x%lx): %d\n",
			(unsigned long long)img_offset, size, ret);
		return ERR_PTR(ret);
	}

	if (!imagemap_record(dev, img_offset, size, dst, false))
		return ERR_PTR(-ENOMEM);

	return dst;
}

void imagemap_cleanup(struct udevice *dev)
{
	struct imagemap_priv *priv;
	struct imagemap_region *r;

	if (!dev)
		return;

	priv = dev_get_uclass_priv(dev);

	alist_for_each(r, &priv->regions) {
		if (r->lmb_reserved)
			lmb_free(map_to_sysmem(r->ram),
				 ALIGN(r->size, ARCH_DMA_MINALIGN),
				 LMB_NONE);
	}

	alist_uninit(&priv->regions);

	device_remove(dev, DM_REMOVE_NORMAL);
	device_unbind(dev);
}

static int imagemap_post_probe(struct udevice *dev)
{
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);

	alist_init_struct(&priv->regions, struct imagemap_region);

	return 0;
}

UCLASS_DRIVER(imagemap) = {
	.id		= UCLASS_IMAGEMAP,
	.name		= "imagemap",
	.post_probe	= imagemap_post_probe,
	.per_device_auto	= sizeof(struct imagemap_priv),
};

/*
 * Block-addressed reader (bl_len == blksz): @off/@size are byte quantities
 * pre-aligned to the block length by spl_load_region()/imagemap, so this is
 * a plain whole-sector transfer.  Returns bytes read, 0 on failure.
 */
static ulong imagemap_blk_reader(struct spl_load_info *info, ulong off,
				 ulong size, void *buf)
{
	struct imagemap_blk *bp = info->priv;
	struct blk_desc *desc = bp->desc;
	lbaint_t blk_off = off >> desc->log2blksz;
	lbaint_t count = size >> desc->log2blksz;

	if (blk_off + count > bp->part_size) {
		log_err("imagemap: read at 0x%lx+0x%lx exceeds partition size\n",
			off, size);
		return 0;
	}

	/*
	 * Re-select our hardware partition / UBI volume in case another
	 * blk access changed it since we were created.  ubi_blk provides no
	 * select_hwpart op, so drive blk_desc->hwpart directly as well.
	 */
	if (desc->hwpart != bp->hwpart) {
		blk_dselect_hwpart(desc, bp->hwpart);
		desc->hwpart = bp->hwpart;
	}

	return blk_dread(desc, bp->part_start + blk_off, count, buf)
	       << desc->log2blksz;
}

static int imagemap_probe(struct udevice *dev)
{
	struct imagemap_plat *plat = dev_get_plat(dev);
	struct imagemap_blk *bp = dev_get_priv(dev);
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);

	bp->desc = dev_get_uclass_plat(dev_get_parent(dev));
	bp->part_start = plat->part_start;
	bp->part_size = plat->part_size;
	bp->hwpart = plat->hwpart;

	spl_load_init(&priv->info, imagemap_blk_reader, bp, bp->desc->blksz);

	return 0;
}

U_BOOT_DRIVER(imagemap) = {
	.name		= "imagemap",
	.id		= UCLASS_IMAGEMAP,
	.probe		= imagemap_probe,
	.plat_auto	= sizeof(struct imagemap_plat),
	.priv_auto	= sizeof(struct imagemap_blk),
};

int imagemap_create(struct udevice *dev, const char *name,
		    int part, struct udevice **devp)
{
	struct blk_desc *desc = dev_get_uclass_plat(dev);
	struct imagemap_plat *plat;
	struct disk_partition info;
	struct udevice *imdev;
	int ret;

	if (name && *name)
		ret = part_get_info_by_name(desc, name, &info);
	else
		ret = part_get_info(desc, part, &info);
	if (ret < 0)
		return ret;

	ret = device_bind_driver(desc->bdev, "imagemap",
				 name && *name ? name : "imagemap", &imdev);
	if (ret)
		return ret;

	plat = dev_get_plat(imdev);
	plat->part_start = info.start;
	plat->part_size = info.size;
	/*
	 * part_get_info_by_name() records the target in blk_desc->hwpart for
	 * ubi_blk (the UBI volume id); capture it so reads re-select it.
	 */
	plat->hwpart = desc->hwpart;

	ret = device_probe(imdev);
	if (ret) {
		device_unbind(imdev);
		return ret;
	}

	*devp = imdev;

	return 0;
}

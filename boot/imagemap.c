// SPDX-License-Identifier: GPL-2.0+
/*
 * On-demand image loading from storage (UCLASS_IMAGEMAP)
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#define LOG_CATEGORY UCLASS_IMAGEMAP

#include <dm.h>
#include <imagemap.h>
#include <linker_lists.h>
#include <lmb.h>
#include <mapmem.h>
#include <asm/cache.h>
#include <dm/device-internal.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <log.h>

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

void *imagemap_map(struct udevice *dev, loff_t img_offset, ulong size)
{
	struct imagemap_priv *priv = dev_get_uclass_priv(dev);
	const struct imagemap_ops *ops = imagemap_get_ops(dev);
	struct imagemap_region *r;
	phys_addr_t addr;
	phys_size_t alloc_size;
	void *p;
	int ret;

	/* Return existing mapping if the range is already covered */
	p = imagemap_lookup(dev, img_offset, size);
	if (p)
		return p;

	alloc_size = ALIGN(size, ARCH_DMA_MINALIGN);

	/*
	 * Check if we have an entry at the same base offset but smaller.
	 * If so, extend the LMB reservation and re-read the full range.
	 */
	alist_for_each(r, &priv->regions) {
		if (r->img_offset == img_offset && r->size < size) {
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
				r->ram = map_sysmem(addr, size);
			}

			ret = ops->read(dev, img_offset, size, r->ram);
			if (ret) {
				log_err("imagemap: read failed at offset 0x%llx (size 0x%lx): %d\n",
					(unsigned long long)img_offset, size, ret);
				lmb_free(addr, alloc_size, LMB_NONE);
				return ERR_PTR(ret);
			}
			r->size = size;
			r->lmb_reserved = true;

			return r->ram;
		}
	}

	/* New region — allocate from LMB */
	if (lmb_alloc_mem(LMB_MEM_ALLOC_ANY, ARCH_DMA_MINALIGN,
			  &addr, alloc_size, LMB_NONE)) {
		log_err("imagemap: LMB alloc failed (0x%lx bytes)\n",
			(ulong)alloc_size);
		return ERR_PTR(-ENOMEM);
	}

	p = map_sysmem(addr, size);

	ret = ops->read(dev, img_offset, size, p);
	if (ret) {
		log_err("imagemap: read failed at offset 0x%llx (size 0x%lx): %d\n",
			(unsigned long long)img_offset, size, ret);
		lmb_free(addr, alloc_size, LMB_NONE);
		return ERR_PTR(ret);
	}

	if (!imagemap_record(dev, img_offset, size, p, true)) {
		lmb_free(addr, alloc_size, LMB_NONE);
		return ERR_PTR(-ENOMEM);
	}

	return p;
}

void *imagemap_map_to(struct udevice *dev, loff_t img_offset,
		      ulong size, void *dst)
{
	const struct imagemap_ops *ops = imagemap_get_ops(dev);
	int ret;

	/* If already mapped to this exact destination, return it */
	void *p = imagemap_lookup(dev, img_offset, size);

	if (p && p == dst)
		return p;

	ret = ops->read(dev, img_offset, size, dst);
	if (ret) {
		log_err("imagemap: read failed at offset 0x%llx (size 0x%lx): %d\n",
			(unsigned long long)img_offset, size, ret);
		return ERR_PTR(ret);
	}

	if (!imagemap_record(dev, img_offset, size, dst, false))
		return ERR_PTR(-ENOMEM);

	return dst;
}

int imagemap_create(struct udevice *dev, const char *name,
		    int part, struct udevice **devp)
{
	struct imagemap_backend *b;
	enum uclass_id id;
	int i, n, ret;

	id = device_get_uclass_id(dev);
	b = ll_entry_start(struct imagemap_backend, imagemap_backend);
	n = ll_entry_count(struct imagemap_backend, imagemap_backend);

	for (i = 0; i < n; i++) {
		if (b[i].uclass == id) {
			ret = b[i].create(dev, name, part, devp);
			if (!ret)
				return 0;
		}
	}

	return -ENODEV;
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

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * On-demand image loading from storage (UCLASS_IMAGEMAP)
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#ifndef __IMAGEMAP_H
#define __IMAGEMAP_H

#include <alist.h>
#include <spl.h>
#include <linux/err.h>
#include <linux/types.h>

struct udevice;

/**
 * struct imagemap_region - One mapped region of the image
 *
 * Records the fact that image bytes [img_offset, img_offset + size)
 * have been loaded into RAM at address @ram.
 *
 * @img_offset:	Start offset within the source image (bytes)
 * @size:	Region size (bytes)
 * @ram:	RAM pointer where this region was loaded
 * @lmb_reserved: true if this region was allocated via LMB and should
 *		be freed on cleanup
 */
struct imagemap_region {
	loff_t img_offset;
	ulong size;
	void *ram;
	bool lmb_reserved;
};

/**
 * struct imagemap_priv - Per-device uclass data for imagemap
 *
 * Managed by the UCLASS_IMAGEMAP uclass via per_device_auto.
 *
 * @info:	Shared load-to-mem abstraction (the same one SPL uses); the
 *		device's probe points its spl_load_reader at the storage,
 *		and all reads go through spl_load_region().
 * @regions:	Translation table of already-loaded regions, used both as
 *		the reuse cache and as the LMB allocation registry (freed on
 *		cleanup).
 */
struct imagemap_priv {
	struct spl_load_info info;
	struct alist regions;
};

/**
 * imagemap_lookup() - Look up an already-mapped region
 *
 * Checks the translation table to see if the requested range
 * [img_offset, img_offset + size) is fully contained within a
 * previously loaded region.
 *
 * @dev:	The imagemap device
 * @img_offset:	Byte offset within the source image
 * @size:	Number of bytes needed
 * Return: RAM pointer on hit, NULL on miss (does not trigger a read)
 */
void *imagemap_lookup(struct udevice *dev, loff_t img_offset, ulong size);

/**
 * imagemap_map() - Ensure an image region is accessible in RAM
 *
 * If the region is already in the translation table, returns the
 * existing RAM pointer. Otherwise allocates RAM via the LMB allocator,
 * reads the data from storage, records the mapping, and returns the
 * new pointer.
 *
 * If the requested range starts at the same offset as an existing
 * region but is larger, the existing region is extended (LMB
 * reservation adjusted, data re-read).
 *
 * @dev:	The imagemap device
 * @img_offset:	Byte offset within the source image
 * @size:	Number of bytes needed
 * Return: RAM pointer on success, ERR_PTR on failure
 */
void *imagemap_map(struct udevice *dev, loff_t img_offset, ulong size);

/**
 * imagemap_map_to() - Load an image region to a specific RAM address
 *
 * Like imagemap_map() but reads into a caller-specified address
 * instead of allocating from the scratch area. Used when the sub-image
 * has a known load address for a zero-copy path.
 *
 * @dev:	The imagemap device
 * @img_offset:	Byte offset within the source image
 * @size:	Number of bytes to load
 * @dst:	Destination address in RAM
 * Return: @dst on success, ERR_PTR on failure
 */
void *imagemap_map_to(struct udevice *dev, loff_t img_offset,
		      ulong size, void *dst);

/**
 * imagemap_cleanup() - Release all resources and unbind the device
 *
 * Frees all LMB reservations from the translation table, removes the
 * driver, and unbinds the device. The device pointer is invalid after
 * this call.
 *
 * Safe to call with a NULL @dev pointer.
 *
 * @dev:	The imagemap device, or NULL
 */
void imagemap_cleanup(struct udevice *dev);

/**
 * imagemap_create() - Create an imagemap device over a block device
 *
 * Resolves a partition on @dev (by name if @name is given, otherwise by
 * index @part), binds an imagemap device as a child of the block device
 * and probes it, pointing its reader at that partition.
 *
 * MTD partitions and UBI volumes are reached the same way: with
 * CONFIG_MTD_BLOCK / CONFIG_UBI_BLOCK they are exposed as named
 * partitions on the mtd_blk / ubi_blk block devices.
 *
 * @dev:	Block device (UCLASS_BLK)
 * @name:	Partition/volume name, or NULL to select by index
 * @part:	Partition index (used when @name is NULL)
 * @devp:	On success, the new imagemap device
 * Return: 0 on success, negative errno on failure
 */
int imagemap_create(struct udevice *dev, const char *name,
		    int part, struct udevice **devp);

#endif /* __IMAGEMAP_H */

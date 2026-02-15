/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * On-demand image loading from storage (UCLASS_IMAGEMAP)
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#ifndef __IMAGEMAP_H
#define __IMAGEMAP_H

#include <alist.h>
#include <dm/uclass-id.h>
#include <linker_lists.h>
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
 * Holds the translation table of already-loaded regions. Managed by
 * the UCLASS_IMAGEMAP uclass via per_device_auto.
 *
 * @regions:	Dynamically growing list of loaded regions
 */
struct imagemap_priv {
	struct alist regions;
};

/**
 * struct imagemap_ops - Driver operations for imagemap backends
 *
 * Each storage backend (block, MTD, UBI) implements this interface.
 */
struct imagemap_ops {
	/**
	 * read() - Read data from a storage device
	 *
	 * @dev:	The imagemap device
	 * @src:	Byte offset within the source image
	 * @size:	Number of bytes to read
	 * @dst:	Destination buffer in RAM
	 * Return: 0 on success, negative errno on failure
	 */
	int (*read)(struct udevice *dev, loff_t src, ulong size, void *dst);
};

#define imagemap_get_ops(dev) \
	((struct imagemap_ops *)(dev)->driver->ops)

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
 * driver (triggering its remove callback for backend cleanup), and
 * unbinds the device. The device pointer is invalid after this call.
 *
 * Safe to call with a NULL @dev pointer.
 *
 * @dev:	The imagemap device, or NULL
 */
void imagemap_cleanup(struct udevice *dev);

/**
 * struct imagemap_backend - Registration for an imagemap storage backend
 *
 * Each backend (block, MTD, UBI) declares one of these via the
 * IMAGEMAP_BACKEND() macro.  imagemap_create() iterates the linker
 * list and calls the first backend whose @uclass matches the device.
 *
 * @uclass:	UCLASS id of the devices this backend handles
 * @create:	Callback to create an imagemap from @dev + @name + @part
 */
struct imagemap_backend {
	enum uclass_id uclass;
	int (*create)(struct udevice *dev, const char *name,
		      int part, struct udevice **devp);
};

#define IMAGEMAP_BACKEND(__name) \
	ll_entry_declare(struct imagemap_backend, __name, imagemap_backend)

/**
 * imagemap_create() - Create an imagemap device for a storage device
 *
 * Iterates registered imagemap backends and selects one whose UCLASS
 * id matches @dev.  The backend creates and probes the imagemap device
 * using @name to identify the partition or volume.
 *
 * For block devices, pass the BLK device and the GPT partition label.
 * For MTD or UBI, pass the MTD device and the partition/volume name.
 *
 * @dev:	Storage device (UCLASS_BLK, UCLASS_MTD, ...)
 * @name:	Partition label, MTD partition name, or UBI volume name
 * @part:	MTD partition index (0-based)
 * @devp:	On success, the new imagemap device
 * Return: 0 on success, negative errno on failure
 */
int imagemap_create(struct udevice *dev, const char *name,
		    int part, struct udevice **devp);

#endif /* __IMAGEMAP_H */

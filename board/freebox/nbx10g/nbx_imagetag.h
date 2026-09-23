/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MVEBU Image Tag header
 *
 * Copyright (C) 2026 Free Mobile, Freebox
 */

#ifndef __MVEBU_IMAGETAG_H
#define __MVEBU_IMAGETAG_H

#include <linux/types.h>

#define MVEBU_IMAGE_TAG_MAGIC	0x8d7c90bc
#define MVEBU_IMAGE_TAG_VERSION	1

/**
 * struct mvebu_image_tag - MVEBU boot image tag structure
 *
 * All multi-byte fields are stored in big-endian format.
 */
struct mvebu_image_tag {
	u32 crc;			/* CRC32-LE checksum (from offset 4) */
	u32 magic;			/* Magic: 0x8d7c90bc */
	u32 version;			/* Version: 1 */
	u32 total_size;			/* Total image size including tag */
	u32 flags;			/* Feature flags (reserved) */

	u32 device_tree_offset;		/* Offset from tag start to DTB */
	u32 device_tree_size;		/* DTB size in bytes */

	u32 kernel_offset;		/* Offset from tag start to kernel */
	u32 kernel_size;		/* Kernel size in bytes */

	u32 rootfs_offset;		/* Offset from tag start to rootfs */
	u32 rootfs_size;		/* Rootfs size (must be 0) */

	char image_name[32];		/* Image name (null-terminated) */
	char build_user[32];		/* Build user info */
	char build_date[32];		/* Build date info */
};

/* Accessor functions for big-endian fields */
static inline u32 mvebu_imagetag_device_tree_offset(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->device_tree_offset);
}

static inline u32 mvebu_imagetag_device_tree_size(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->device_tree_size);
}

static inline u32 mvebu_imagetag_kernel_offset(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->kernel_offset);
}

static inline u32 mvebu_imagetag_kernel_size(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->kernel_size);
}

static inline u32 mvebu_imagetag_rootfs_offset(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->rootfs_offset);
}

static inline u32 mvebu_imagetag_rootfs_size(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->rootfs_size);
}

static inline u32 mvebu_imagetag_total_size(struct mvebu_image_tag *tag)
{
	return be32_to_cpu(tag->total_size);
}

#endif /* __MVEBU_IMAGETAG_H */

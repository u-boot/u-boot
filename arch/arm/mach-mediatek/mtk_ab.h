/* SPDX-License-Identifier: GPL-2.0-or-later AND BSD-3-Clause */
/*
 * Copyright (c) 2023, Mediatek Inc. All rights reserved.
 * Copyright (c) 2026 BayLibre SAS
 */

#ifndef __MTK_AB_H
#define __MTK_AB_H

#include <linux/types.h>

struct blk_desc;

#define MTK_AB_MAGIC	0x544f494d
#define MTK_AB_VERSION	1
#define MTK_AB_MAX_TRIES	9

/* MediaTek v1 disk ABI, including the original AArch64 structure padding. */
struct mtk_ab_control {
	__le32 slot;
	__le32 magic;
	u8 version;
	u8 tries[2];
	u8 padding;
	u8 reserved[12];
	__le32 crc32;
};

/**
 * mtk_ab_select() - Select and account for a MediaTek firmware slot
 * @desc: Block device containing the GPT user area
 * @slot: Returns the selected slot (0 for A, 1 for B), only on success
 *
 * Resolve bootloaders, bootloaders_b and misc by name, validate the v1 record
 * at the start of misc, and persist any required retry update before returning.
 * The caller must provide a valid, non-overlapping GPT layout.
 * A zero counter denotes success; counters 1..8 consume a trial and 9 is
 * exhausted. Only an all-zero or all-0xff record is initialized automatically.
 * Writes preserve the rest of the block and are verified by reading back
 * with U-Boot's block cache invalidated. This single-copy format does not
 * provide power-fail atomicity.
 *
 * Return: Positive GPT partition number to boot, or a negative error. The
 * caller must stop booting on error; a failed write may have changed storage.
 */
int mtk_ab_select(struct blk_desc *desc, unsigned int *slot);

#endif

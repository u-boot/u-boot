// SPDX-License-Identifier: GPL-2.0-or-later AND BSD-3-Clause
/*
 * Copyright (c) 2023, Mediatek Inc. All rights reserved.
 * Copyright (c) 2026 BayLibre SAS
 *
 * Disk format and valid-record transitions follow MediaTek's TF-A mtk_ab.c.
 */

#include <blk.h>
#include <malloc.h>
#include <memalign.h>
#include <part.h>
#include <stdio.h>
#include <asm/byteorder.h>
#include <linux/build_bug.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/log2.h>
#include <linux/string.h>
#include <u-boot/crc.h>

#include "mtk_ab.h"

#define MTK_AB_MISC_INDEX	2

static_assert(sizeof(struct mtk_ab_control) == 28);
static_assert(offsetof(struct mtk_ab_control, padding) == 11);
static_assert(offsetof(struct mtk_ab_control, reserved) == 12);
static_assert(offsetof(struct mtk_ab_control, crc32) == 24);

static u32 mtk_ab_crc(const struct mtk_ab_control *ctrl)
{
	return crc32(0, (const void *)ctrl, offsetof(struct mtk_ab_control, crc32));
}

static int mtk_ab_layout(struct blk_desc *desc, struct disk_partition *parts,
			 int *numbers)
{
	static const char * const names[] = { "bootloaders", "bootloaders_b", "misc" };
	int i;

	part_init(desc);
	if (desc->part_type != PART_TYPE_EFI)
		return -EPROTONOSUPPORT;

	for (i = 0; i < ARRAY_SIZE(names); i++) {
		numbers[i] = part_get_info_by_name(desc, names[i], &parts[i]);
		if (numbers[i] < 0)
			return numbers[i];
	}

	/* The metadata read-modify-write needs one block in misc. */
	if (!parts[MTK_AB_MISC_INDEX].start || !parts[MTK_AB_MISC_INDEX].size ||
	    parts[MTK_AB_MISC_INDEX].start >= desc->lba)
		return -EINVAL;

	return 0;
}

int mtk_ab_select(struct blk_desc *desc, unsigned int *slot)
{
	struct disk_partition parts[3];
	struct mtk_ab_control *ctrl;
	unsigned int selected;
	int numbers[3], ret;
	void *block, *verify;
	bool changed = false;

	if (!desc || !slot || !desc->lba || desc->hwpart || desc->blksz < 512 ||
	    desc->blksz > 4096 || !is_power_of_2(desc->blksz) ||
	    desc->blksz % ARCH_DMA_MINALIGN)
		return -EINVAL;

	/* Locate all required partitions before updating the metadata. */
	ret = mtk_ab_layout(desc, parts, numbers);
	if (ret)
		return ret;

	block = malloc_cache_aligned(2 * desc->blksz);
	if (!block)
		return -ENOMEM;
	verify = block + desc->blksz;
	ctrl = block;

	ret = -EIO;
	if (blk_dread(desc, parts[MTK_AB_MISC_INDEX].start, 1, block) != 1)
		goto out;

	if (!memchr_inv(ctrl, 0, sizeof(*ctrl)) ||
	    !memchr_inv(ctrl, 0xff, sizeof(*ctrl))) {
		puts("MediaTek A/B: initializing blank metadata\n");
		memset(ctrl, 0, sizeof(*ctrl));
		ctrl->magic = cpu_to_le32(MTK_AB_MAGIC);
		ctrl->version = MTK_AB_VERSION;
		ctrl->tries[0] = 1;
		ctrl->tries[1] = 1;
		selected = 0;
		changed = true;
	} else {
		ret = -EINVAL;
		selected = le32_to_cpu(ctrl->slot);
		if (le32_to_cpu(ctrl->magic) != MTK_AB_MAGIC ||
		    ctrl->version != MTK_AB_VERSION || selected > 1 ||
		    ctrl->tries[0] > MTK_AB_MAX_TRIES ||
		    ctrl->tries[1] > MTK_AB_MAX_TRIES)
			goto out;
		ret = -EBADMSG;
		if (le32_to_cpu(ctrl->crc32) != mtk_ab_crc(ctrl))
			goto out;

		if (ctrl->tries[selected] == MTK_AB_MAX_TRIES) {
			selected ^= 1;
			ret = -ENOENT;
			if (ctrl->tries[selected] == MTK_AB_MAX_TRIES)
				goto out;
			ctrl->slot = cpu_to_le32(selected);
			changed = true;
		}
		if (ctrl->tries[selected]) {
			ctrl->tries[selected]++;
			changed = true;
		}
	}

	if (changed) {
		ctrl->crc32 = cpu_to_le32(mtk_ab_crc(ctrl));
		ret = -EIO;
		if (blk_dwrite(desc, parts[MTK_AB_MISC_INDEX].start, 1, block) != 1)
			goto out;
		if (blk_dread(desc, parts[MTK_AB_MISC_INDEX].start, 1, verify) != 1 ||
		    memcmp(block, verify, desc->blksz))
			goto out;
	}

	*slot = selected;
	ret = numbers[selected];
out:
	free(block);
	return ret;
}

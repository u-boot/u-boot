// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2024 SaluteDevices, Inc.
 *
 * Author: Alexey Romanov <avromanov@salutedevices.com>
 */

#include <memalign.h>
#include <part.h>
#include <ubi_uboot.h>

static inline struct ubi_device *get_ubi_device(void)
{
	return ubi_devices[0];
}

static struct ubi_volume *ubi_get_volume_by_index(int vol_id)
{
	struct ubi_device *ubi = get_ubi_device();
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		struct ubi_volume *volume = ubi->volumes[i];

		if (!volume)
			continue;

		if (volume->vol_id >= UBI_INTERNAL_VOL_START)
			continue;

		if (volume->vol_id == vol_id)
			return volume;
	}

	return NULL;
}

static int __maybe_unused part_get_info_ubi(struct blk_desc *dev_desc, int part_idx,
					    struct disk_partition *info)
{
	struct ubi_volume *vol;

	/*
	 * We must use part_idx - 1 instead of part_idx, because
	 * part_get_info_by_name() start indexing at 1, not 0.
	 * ubi volumes idexed starting at 0
	 */
	vol = ubi_get_volume_by_index(part_idx - 1);
	if (!vol)
		return -ENOENT;

	snprintf(info->name, PART_NAME_LEN, vol->name);

	info->start = 0;
	info->size = (unsigned long)vol->used_bytes / dev_desc->blksz;
	info->blksz = dev_desc->blksz;

	/* Save UBI volume ID in blk device descriptor */
	dev_desc->hwpart = vol->vol_id;

	return 0;
}

static void __maybe_unused part_print_ubi(struct blk_desc *dev_desc)
{
	struct ubi_device *ubi = get_ubi_device();
	int i;

	for (i = 0; i < (ubi->vtbl_slots + 1); i++) {
		struct ubi_volume *volume = ubi->volumes[i];

		if (!volume)
			continue;

		if (volume->vol_id >= UBI_INTERNAL_VOL_START)
			continue;

		printf("%d: %s\n", volume->vol_id, volume->name);
	}
}

static int part_test_ubi(struct blk_desc *dev_desc)
{
	ALLOC_CACHE_ALIGN_BUFFER(unsigned char, buffer, dev_desc->blksz);

	if (blk_dread(dev_desc, 0, 1, (ulong *)buffer) != 1)
		return -1;

	return 0;
}

U_BOOT_PART_TYPE(ubi) = {
	.name	= "ubi",
	.part_type	= PART_TYPE_UBI,
	.max_entries	= UBI_ENTRY_NUMBERS,
	.get_info	= part_get_info_ptr(part_get_info_ubi),
	.print	= part_print_ptr(part_print_ubi),
	.test	= part_test_ubi,
};

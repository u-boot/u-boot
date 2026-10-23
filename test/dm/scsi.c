// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <dm.h>
#include <part.h>
#include <scsi.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that sandbox SCSI works correctly */
static int dm_test_scsi_base(struct unit_test_state *uts)
{
	const struct disk_partition *info;
	const struct disk_part *part;
	struct udevice *dev;

	ut_assertok(scsi_scan(false));

	/*
	 * We expect some sort of partition on the disk image, created by
	 * test_ut_dm_init()
	 */
	ut_assertok(uclass_first_device_err(UCLASS_PARTITION, &dev));

	part = dev_get_uclass_plat(dev);
	ut_asserteq(1, part->partnum);

	info = &part->gpt_part_info;
	ut_asserteq_str("sda1", info->name);
	ut_asserteq_str("0x83", info->type);
	ut_asserteq(0x83 /* linux */, info->sys_ind);

	return 0;
}
DM_TEST(dm_test_scsi_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test that a disk reporting an unsupported block size is refused */
static int dm_test_scsi_blksz(struct unit_test_state *uts)
{
	static const struct {
		int blksz;
		bool ok;
	} cases[] = {
		{ 256, false },
		{ 520, false },
		{ 2 * BLK_MAX_BLKSZ, false },
		{ SZ_4K, true },
	};
	struct udevice *dev, *blk;
	struct blk_desc *desc;
	int i, ret;

	ut_assertok(uclass_first_device_err(UCLASS_SCSI, &dev));

	for (i = 0; i < ARRAY_SIZE(cases); i++) {
		sandbox_scsi_set_block_size(dev, cases[i].blksz);
		ut_assertok(scsi_scan(false));

		ret = device_find_first_child_by_uclass(dev, UCLASS_BLK, &blk);
		if (!cases[i].ok) {
			ut_asserteq(-ENODEV, ret);
			continue;
		}
		ut_assertok(ret);
		desc = dev_get_uclass_plat(blk);
		ut_asserteq(cases[i].blksz, desc->blksz);
	}

	return 0;
}
DM_TEST(dm_test_scsi_blksz, UTF_SCAN_PDATA | UTF_SCAN_FDT);

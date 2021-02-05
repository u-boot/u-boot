// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <fastboot.h>
#include <fb_mmc.h>
#include <mmc.h>
#include <part.h>
#include <part_efi.h>
#include <dm/test.h>
#include <test/ut.h>
#include <linux/stringify.h>

#define FB_ALIAS_PREFIX "fastboot_partition_alias_"

static int dm_test_fastboot_mmc_part(struct unit_test_state *uts)
{
	char response[FASTBOOT_RESPONSE_LEN] = {0};
	char str_disk_guid[UUID_STR_LEN + 1];
	struct blk_desc *mmc_dev_desc, *fb_dev_desc;
	struct disk_partition part_info;
	struct disk_partition parts[2] = {
		{
			.start = 48, /* GPT data takes up the first 34 blocks or so */
			.size = 1,
			.name = "test1",
		},
		{
			.start = 49,
			.size = 1,
			.name = "test2",
		},
	};

	ut_assertok(blk_get_device_by_str("mmc",
					  __stringify(CONFIG_FASTBOOT_FLASH_MMC_DEV),
					  &mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(mmc_dev_desc, str_disk_guid, parts,
				ARRAY_SIZE(parts)));

	/* "Classic" partition labels */
	ut_asserteq(1, fastboot_mmc_get_part_info("test1", &fb_dev_desc,
						  &part_info, response));
	ut_asserteq(2, fastboot_mmc_get_part_info("test2", &fb_dev_desc,
						  &part_info, response));

	/* Test aliases */
	ut_assertnull(env_get(FB_ALIAS_PREFIX "test3"));
	ut_assertok(env_set(FB_ALIAS_PREFIX "test3", "test1"));
	ut_asserteq(1, fastboot_mmc_get_part_info("test3", &fb_dev_desc,
						  &part_info, response));
	ut_assertok(env_set(FB_ALIAS_PREFIX "test3", NULL));

	return 0;
}
DM_TEST(dm_test_fastboot_mmc_part, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

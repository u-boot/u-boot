// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Sean Anderson <sean.anderson@seco.com>
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <part.h>
#include <part_efi.h>
#include <dm/test.h>
#include <test/ut.h>

static inline int do_test(struct unit_test_state *uts, int expected,
			  const char *part_str, bool whole)
{
	struct blk_desc *mmc_dev_desc;
	struct disk_partition part_info;

	ut_asserteq(expected,
		    part_get_info_by_dev_and_name_or_num("mmc", part_str,
							 &mmc_dev_desc,
							 &part_info, whole));
	return 0;
}

static int dm_test_part(struct unit_test_state *uts)
{
	char *oldbootdevice;
	char str_disk_guid[UUID_STR_LEN + 1];
	int ret;
	struct blk_desc *mmc_dev_desc;
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

	ut_asserteq(1, blk_get_device_by_str("mmc", "1", &mmc_dev_desc));
	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		gen_rand_uuid_str(parts[0].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(parts[1].uuid, UUID_STR_FORMAT_STD);
		gen_rand_uuid_str(str_disk_guid, UUID_STR_FORMAT_STD);
	}
	ut_assertok(gpt_restore(mmc_dev_desc, str_disk_guid, parts,
				ARRAY_SIZE(parts)));

	oldbootdevice = env_get("bootdevice");

#define test(expected, part_str, whole) do { \
	ret = do_test(uts, expected, part_str, whole); \
	if (ret) \
		goto out; \
} while (0)

	env_set("bootdevice", NULL);
	test(-ENODEV, NULL, true);
	test(-ENODEV, "", true);
	env_set("bootdevice", "0");
	test(0, NULL, true);
	test(0, "", true);
	env_set("bootdevice", "1");
	test(1, NULL, false);
	test(1, "", false);
	test(1, "-", false);
	env_set("bootdevice", "");
	test(-EPROTONOSUPPORT, "0", false);
	test(0, "0", true);
	test(0, ":0", true);
	test(0, ".0", true);
	test(0, ".0:0", true);
	test(-EINVAL, "#test1", true);
	test(1, "1", false);
	test(1, "1", true);
	test(-ENOENT, "1:0", false);
	test(0, "1:0", true);
	test(1, "1:1", false);
	test(2, "1:2", false);
	test(1, "1.0", false);
	test(0, "1.0:0", true);
	test(1, "1.0:1", false);
	test(2, "1.0:2", false);
	test(-EINVAL, "1#bogus", false);
	test(1, "1#test1", false);
	test(2, "1#test2", false);
	ret = 0;

out:
	env_set("bootdevice", oldbootdevice);
	return ret;
}
DM_TEST(dm_test_part, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

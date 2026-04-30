// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for part command
 *
 * Copyright (C) 2026 Amarula Solutions
 * Written by Dario Binacchi <dario.binacchi@amarulasolutions.com>
 */

#include <command.h>
#include <dm.h>
#include <env.h>
#include <part.h>
#include <vsprintf.h>
#include <dm/test.h>
#include <test/cmd.h>
#include <test/test.h>
#include <test/ut.h>

static struct disk_partition gpt_parts[] = {
	{
		.start = 48,
		.size = 1,
		.name = "test1",
		.uuid = "c5bce7a2-03f0-4d03-9048-01ff23b9d527",
	},
	{
		.start = 49,
		.size = 2,
		.name = "test2",
		.uuid = "9df346e8-2c53-4cd8-b9ac-3af83f9a9b74",
	},
};

static char disk_guid[UUID_STR_LEN + 1] =
	"8d60b397-1bb6-4d33-80ee-b1587d24c2f8";

static int setup_gpt_partitions(struct unit_test_state *uts,
				unsigned int mmc_dev_num)
{
	struct blk_desc *mmc_dev_desc;
	char dev_str[10];
	int i, ret;

	if (!CONFIG_IS_ENABLED(MMC))
		return -EAGAIN;

	snprintf(dev_str, sizeof(dev_str), "%u", mmc_dev_num);

	ret = blk_get_device_by_str("mmc", dev_str, &mmc_dev_desc);
	if (ret == -ENODEV)
		return -EAGAIN;

	ut_asserteq(mmc_dev_num, ret);

	if (CONFIG_IS_ENABLED(RANDOM_UUID)) {
		for (i = 0; i < ARRAY_SIZE(gpt_parts); i++)
			gen_rand_uuid_str(gpt_parts[i].uuid,
					  UUID_STR_FORMAT_STD);

		gen_rand_uuid_str(disk_guid, UUID_STR_FORMAT_STD);
	}

	ut_assertok(gpt_restore(mmc_dev_desc, disk_guid, gpt_parts,
				ARRAY_SIZE(gpt_parts)));
	return 0;
}

static int cmd_test_part_number(struct unit_test_state *uts)
{
	unsigned int mmc_dev_num = 2;
	char expected[10];
	int i, ret;

	ret = setup_gpt_partitions(uts, mmc_dev_num);
	if (ret == -EAGAIN)
		return ret;

	ut_assertok(ret);

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partnum", NULL);
		ut_assertok(run_commandf("part number mmc %u %s partnum",
					 mmc_dev_num, gpt_parts[i].name));
		snprintf(expected, sizeof(expected), "0x%x", i + 1);
		ut_asserteq_str(expected, env_get("partnum"));
	}

	env_set("partnum", NULL);
	ut_asserteq(1, run_commandf("part number mmc %u bogus partnum",
				    mmc_dev_num));
	ut_assertnull(env_get("partnum"));

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partnum", NULL);
		ut_assertok(run_commandf("part number mmc %u %s partnum",
					 mmc_dev_num, gpt_parts[i].uuid));
		snprintf(expected, sizeof(expected), "0x%x", i + 1);
		ut_asserteq_str(expected, env_get("partnum"));
	}

	env_set("partnum", NULL);
	ut_asserteq(1, run_commandf("part number mmc %u %s partnum",
				    mmc_dev_num,
				    "00000000-0000-0000-0000-000000000000"));
	ut_assertnull(env_get("partnum"));

	return 0;
}
CMD_TEST(cmd_test_part_number, UTF_CONSOLE);

static int cmd_test_part_start(struct unit_test_state *uts)
{
	unsigned int mmc_dev_num = 2;
	char expected[32];
	int i, ret;

	ret = setup_gpt_partitions(uts, mmc_dev_num);
	if (ret == -EAGAIN)
		return ret;

	ut_assertok(ret);

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partstart", NULL);
		ut_assertok(run_commandf("part start mmc %u %d partstart",
					 mmc_dev_num, i + 1));
		snprintf(expected, sizeof(expected), "%lx",
			 (unsigned long)gpt_parts[i].start);
		ut_asserteq_str(expected, env_get("partstart"));
	}

	env_set("partstart", NULL);
	ut_asserteq(1, run_commandf("part start mmc %u 3 partstart",
				    mmc_dev_num));
	ut_assertnull(env_get("partstart"));

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partstart", NULL);
		ut_assertok(run_commandf("part start mmc %u %s partstart",
					 mmc_dev_num, gpt_parts[i].name));
		snprintf(expected, sizeof(expected), "%lx",
			 (unsigned long)gpt_parts[i].start);
		ut_asserteq_str(expected, env_get("partstart"));
	}

	env_set("partstart", NULL);
	ut_asserteq(1, run_commandf("part start mmc %u bogus partstart",
				    mmc_dev_num));
	ut_assertnull(env_get("partstart"));

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partstart", NULL);
		ut_assertok(run_commandf("part start mmc %u %s partstart",
					 mmc_dev_num, gpt_parts[i].uuid));
		snprintf(expected, sizeof(expected), "%lx",
			 (unsigned long)gpt_parts[i].start);
		ut_asserteq_str(expected, env_get("partstart"));
	}

	env_set("partstart", NULL);
	ut_asserteq(1, run_commandf("part start mmc %u %s partstart",
				    mmc_dev_num,
				    "00000000-0000-0000-0000-000000000000"));
	ut_assertnull(env_get("partstart"));

	return 0;
}
CMD_TEST(cmd_test_part_start, UTF_CONSOLE);

static int cmd_test_part_size(struct unit_test_state *uts)
{
	unsigned int mmc_dev_num = 2;
	char expected[32];
	int i, ret;

	ret = setup_gpt_partitions(uts, mmc_dev_num);
	if (ret == -EAGAIN)
		return ret;

	ut_assertok(ret);

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partsize", NULL);
		ut_assertok(run_commandf("part size mmc %u %d partsize",
					 mmc_dev_num, i + 1));
		snprintf(expected, sizeof(expected), "%lx",
			 (unsigned long)gpt_parts[i].size);
		ut_asserteq_str(expected, env_get("partsize"));
	}

	env_set("partsize", NULL);
	ut_asserteq(1, run_commandf("part size mmc %u 3 partsize",
				    mmc_dev_num));
	ut_assertnull(env_get("partsize"));

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partsize", NULL);
		ut_assertok(run_commandf("part size mmc %u %s partsize",
					 mmc_dev_num, gpt_parts[i].name));
		snprintf(expected, sizeof(expected), "%lx",
			 (unsigned long)gpt_parts[i].size);
		ut_asserteq_str(expected, env_get("partsize"));
	}

	env_set("partsize", NULL);
	ut_asserteq(1, run_commandf("part size mmc %u bogus partsize",
				    mmc_dev_num));
	ut_assertnull(env_get("partsize"));

	for (i = 0; i < ARRAY_SIZE(gpt_parts); i++) {
		env_set("partsize", NULL);
		ut_assertok(run_commandf("part size mmc %u %s partsize",
					 mmc_dev_num, gpt_parts[i].uuid));
		snprintf(expected, sizeof(expected), "%lx",
			 (unsigned long)gpt_parts[i].size);
		ut_asserteq_str(expected, env_get("partsize"));
	}

	env_set("partsize", NULL);
	ut_asserteq(1, run_commandf("part size mmc %u %s partsize",
				    mmc_dev_num,
				    "00000000-0000-0000-0000-000000000000"));
	ut_assertnull(env_get("partsize"));

	return 0;
}
CMD_TEST(cmd_test_part_size, UTF_CONSOLE);

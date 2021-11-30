// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <part.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/*
 * Basic test of the mmc uclass. We could expand this by implementing an MMC
 * stack for sandbox, or at least implementing the basic operation.
 */
static int dm_test_mmc_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));

	return 0;
}
DM_TEST(dm_test_mmc_base, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

static int dm_test_mmc_blk(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct blk_desc *dev_desc;
	int i;
	char write[1024], read[1024];

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	ut_assertok(blk_get_device_by_str("mmc", "0", &dev_desc));

	/* Write a few blocks and verify that we get the same data back */
	ut_asserteq(512, dev_desc->blksz);
	for (i = 0; i < sizeof(write); i++)
		write[i] = i;
	ut_asserteq(2, blk_dwrite(dev_desc, 0, 2, write));
	ut_asserteq(2, blk_dread(dev_desc, 0, 2, read));
	ut_asserteq_mem(write, read, sizeof(write));

	/* Now erase them */
	memset(write, '\0', sizeof(write));
	ut_asserteq(2, blk_derase(dev_desc, 0, 2));
	ut_asserteq(2, blk_dread(dev_desc, 0, 2, read));
	ut_asserteq_mem(write, read, sizeof(write));

	return 0;
}
DM_TEST(dm_test_mmc_blk, UT_TESTF_SCAN_PDATA | UT_TESTF_SCAN_FDT);

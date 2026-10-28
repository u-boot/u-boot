// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <dm.h>
#include <mmc.h>
#include <part.h>
#include <asm/test.h>
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
DM_TEST(dm_test_mmc_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

static int dm_test_mmc_blk(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct blk_desc *dev_desc;
	int i;
	char write[4 * 512], read[4 * 512];

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));
	ut_assertok(blk_get_device_by_str("mmc", "0", &dev_desc));

	/* Write a few blocks and verify that we get the same data back */
	ut_asserteq(512, dev_desc->blksz);
	for (i = 0; i < sizeof(write); i++)
		write[i] = i;
	ut_asserteq(4, blk_dwrite(dev_desc, 0, 4, write));
	ut_asserteq(4, blk_dread(dev_desc, 0, 4, read));
	ut_asserteq_mem(write, read, sizeof(write));

	/* Now erase two of them [1 - 2] and verify all blocks */
	memset(&write[512], '\0', 2 * 512);
	ut_asserteq(2, blk_derase(dev_desc, 1, 2));
	ut_asserteq(4, blk_dread(dev_desc, 0, 4, read));
	ut_asserteq_mem(write, read, sizeof(write));

	return 0;
}
DM_TEST(dm_test_mmc_blk, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test that a card with a READ_BL_LEN other than 9 gets 512-byte blocks */
static int dm_test_mmc_bl_len(struct unit_test_state *uts)
{
	static const int shift[] = { 0, 8, 11 };
	struct blk_desc *dev_desc;
	char write[512], read[512];
	struct udevice *dev;
	struct mmc *mmc;
	int i, j;

	ut_assertok(uclass_get_device_by_seq(UCLASS_MMC, 0, &dev));
	ut_assertok(blk_get_device_by_str("mmc", "0", &dev_desc));
	mmc = mmc_get_mmc_dev(dev);

	for (i = 0; i < ARRAY_SIZE(shift); i++) {
		/* Initialise the card again, as 'mmc rescan' does */
		sandbox_mmc_set_bl_len_shift(dev, shift[i]);
		mmc->has_init = 0;
		ut_assertok(mmc_init(mmc));

		ut_asserteq(512, mmc->read_bl_len);
		ut_asserteq(512, mmc->write_bl_len);
		ut_asserteq(512, dev_desc->blksz);
		ut_asserteq(9, dev_desc->log2blksz);

		/* mmc0 reports 1024 blocks of the length in READ_BL_LEN */
		ut_asserteq((1024ULL << shift[i]) / 512, dev_desc->lba);

		for (j = 0; j < sizeof(write); j++)
			write[j] = i + j;
		ut_asserteq(1, blk_dwrite(dev_desc, 1, 1, write));
		ut_asserteq(1, blk_dread(dev_desc, 1, 1, read));
		ut_asserteq_mem(write, read, sizeof(write));
	}

	return 0;
}
DM_TEST(dm_test_mmc_bl_len, UTF_SCAN_PDATA | UTF_SCAN_FDT);

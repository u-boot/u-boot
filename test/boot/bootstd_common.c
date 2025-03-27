// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for bootdev functions. All start with 'bootdev'
 *
 * Copyright 2021 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <bootdev.h>
#include <bootstd.h>
#include <dm.h>
#include <memalign.h>
#include <mmc.h>
#include <usb.h>
#include <linux/log2.h>
#include <test/ut.h>
#include <u-boot/crc.h>
#include "bootstd_common.h"

/* tracks whether bootstd_setup_for_tests() has been run yet */
bool vbe_setup_done;

/**
 * bootstd_setup_for_tests() - Set up MMC data for VBE tests
 *
 * Some data is needed for VBE tests to work. This function sets that up.
 *
 * @return 0 if OK, -ve on error
 */
static int bootstd_setup_for_tests(struct unit_test_state *uts)
{
	ALLOC_CACHE_ALIGN_BUFFER(u8, buf, MMC_MAX_BLOCK_LEN);
	struct udevice *mmc;
	struct blk_desc *desc;
	int ret;

	if (vbe_setup_done)
		return 0;

	/* Set up the version string */
	ret = uclass_get_device(UCLASS_MMC, 1, &mmc);
	if (ret)
		return log_msg_ret("mmc", -EIO);
	desc = blk_get_by_device(mmc);

	memset(buf, '\0', MMC_MAX_BLOCK_LEN);
	strcpy(buf, TEST_VERSION);
	if (blk_dwrite(desc, VERSION_START_BLK, 1, buf) != 1)
		return log_msg_ret("wr1", -EIO);

	/* Set up the nvdata */
	memset(buf, '\0', MMC_MAX_BLOCK_LEN);
	buf[1] = ilog2(0x40) << 4 | 1;
	*(u32 *)(buf + 4) = TEST_VERNUM;
	buf[0] = crc8(0, buf + 1, 0x3f);
	if (blk_dwrite(desc, NVDATA_START_BLK, 1, buf) != 1)
		return log_msg_ret("wr2", -EIO);

	vbe_setup_done = true;

	return 0;
}
BOOTSTD_TEST_INIT(bootstd_setup_for_tests, 0);

int bootstd_test_drop_bootdev_order(struct unit_test_state *uts)
{
	struct bootstd_priv *priv;
	struct udevice *bootstd;

	ut_assertok(uclass_first_device_err(UCLASS_BOOTSTD, &bootstd));
	priv = dev_get_priv(bootstd);
	priv->bootdev_order = NULL;

	return 0;
}

int bootstd_test_check_mmc_hunter(struct unit_test_state *uts)
{
	struct bootdev_hunter *start, *mmc;
	struct bootstd_priv *std;
	uint seq;

	if (!IS_ENABLED(CONFIG_MMC))
		return 0;

	/* get access to the used hunters */
	ut_assertok(bootstd_get_priv(&std));

	/* check that the hunter was used */
	start = ll_entry_start(struct bootdev_hunter, bootdev_hunter);
	mmc = BOOTDEV_HUNTER_GET(mmc_bootdev_hunter);
	seq = mmc - start;
	ut_asserteq(BIT(seq), std->hunters_used);

	return 0;
}

void bootstd_reset_usb(void)
{
	usb_started = false;
}

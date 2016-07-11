/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <asm/state.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test that block devices can be created */
static int dm_test_blk_base(struct unit_test_state *uts)
{
	struct udevice *blk, *usb_blk, *dev;

	/* Make sure there are no block devices */
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_BLK, 0, &blk));

	/* Create two, one the parent of the other */
	ut_assertok(blk_create_device(gd->dm_root, "sandbox_host_blk", "test",
				      IF_TYPE_HOST, 1, 512, 1024, &blk));
	ut_assertok(blk_create_device(blk, "usb_storage_blk", "test",
				      IF_TYPE_USB, 3, 512, 1024, &usb_blk));

	/* Check we can find them */
	ut_asserteq(-ENODEV, blk_get_device(IF_TYPE_HOST, 0, &dev));
	ut_assertok(blk_get_device(IF_TYPE_HOST, 1, &dev));
	ut_asserteq_ptr(blk, dev);

	ut_asserteq(-ENODEV, blk_get_device(IF_TYPE_USB, 0, &dev));
	ut_assertok(blk_get_device(IF_TYPE_USB, 3, &dev));
	ut_asserteq_ptr(usb_blk, dev);

	/* Check we can iterate */
	ut_assertok(blk_first_device(IF_TYPE_HOST, &dev));
	ut_asserteq_ptr(blk, dev);
	ut_asserteq(-ENODEV, blk_next_device(&dev));

	ut_assertok(blk_first_device(IF_TYPE_USB, &dev));
	ut_asserteq_ptr(usb_blk, dev);
	ut_asserteq(-ENODEV, blk_next_device(&dev));

	return 0;
}
DM_TEST(dm_test_blk_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

static int count_blk_devices(void)
{
	struct udevice *blk;
	struct uclass *uc;
	int count = 0;
	int ret;

	ret = uclass_get(UCLASS_BLK, &uc);
	if (ret)
		return ret;

	uclass_foreach_dev(blk, uc)
		count++;

	return count;
}

/* Test that block devices work correctly with USB */
static int dm_test_blk_usb(struct unit_test_state *uts)
{
	struct udevice *usb_dev, *dev;
	struct blk_desc *dev_desc;

	/* Get a flash device */
	state_set_skip_delays(true);
	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &usb_dev));
	ut_assertok(blk_get_device_by_str("usb", "0", &dev_desc));

	/* The parent should be a block device */
	ut_assertok(blk_get_device(IF_TYPE_USB, 0, &dev));
	ut_asserteq_ptr(usb_dev, dev_get_parent(dev));

	/* Check we have one block device for each mass storage device */
	ut_asserteq(3, count_blk_devices());

	/* Now go around again, making sure the old devices were unbound */
	ut_assertok(usb_stop());
	ut_assertok(usb_init());
	ut_asserteq(3, count_blk_devices());
	ut_assertok(usb_stop());

	return 0;
}
DM_TEST(dm_test_blk_usb, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

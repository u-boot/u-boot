/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <usb.h>
#include <asm/io.h>
#include <dm/test.h>
#include <test/ut.h>

/* Test that sandbox USB works correctly */
static int dm_test_usb_base(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_USB, 0, &bus));
	ut_assertok(uclass_get_device(UCLASS_USB, 0, &bus));
	ut_asserteq(-ENODEV, uclass_get_device_by_seq(UCLASS_USB, 2, &bus));

	return 0;
}
DM_TEST(dm_test_usb_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/*
 * Test that we can use the flash stick. This is more of a functional test. It
 * covers scanning the bug, setting up a hub and a flash stick and reading
 * data from the flash stick.
 */
static int dm_test_usb_flash(struct unit_test_state *uts)
{
	struct udevice *dev;
	block_dev_desc_t *dev_desc;
	char cmp[1024];

	ut_assertok(usb_init());
	ut_assertok(uclass_get_device(UCLASS_MASS_STORAGE, 0, &dev));
	ut_assertok(get_device("usb", "0", &dev_desc));

	/* Read a few blocks and look for the string we expect */
	ut_asserteq(512, dev_desc->blksz);
	memset(cmp, '\0', sizeof(cmp));
	ut_asserteq(2, dev_desc->block_read(dev_desc->dev, 0, 2, cmp));
	ut_assertok(strcmp(cmp, "this is a test"));

	return 0;
}
DM_TEST(dm_test_usb_flash, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

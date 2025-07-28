// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025 Altera Corporation <www.altera.com>
 */

#include <dm.h>
#include <i3c.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Basic test of the i3c uclass */
static int dm_test_i3c_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_I3C, 0, &dev));
	ut_assertok(dm_i3c_read(dev, 0, NULL, 1));
	ut_assertok(dm_i3c_read(dev, 0, NULL, 4));
	ut_assertok(dm_i3c_write(dev, 0, "AABB", 2));
	ut_assertok(dm_i3c_write(dev, 0, "AABBCCDD", 4));

	ut_assertok(uclass_get_device(UCLASS_I3C, 1, &dev));
	ut_assertok(dm_i3c_read(dev, 1, NULL, 1));
	ut_assertok(dm_i3c_read(dev, 1, NULL, 4));
	ut_assertok(dm_i3c_write(dev, 1, "AABB", 2));
	ut_assertok(dm_i3c_write(dev, 1, "AABBCCDD", 4));

	ut_asserteq(-ENODEV, uclass_get_device(UCLASS_I3C, 2, &dev));

	return 0;
}
DM_TEST(dm_test_i3c_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

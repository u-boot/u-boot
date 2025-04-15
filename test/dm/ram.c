// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <dm.h>
#include <ram.h>
#include <asm/global_data.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Basic test of the ram uclass */
static int dm_test_ram_base(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct ram_info info;

	ut_assertok(uclass_get_device(UCLASS_RAM, 0, &dev));
	ut_assertok(ram_get_info(dev, &info));
	ut_asserteq(0, info.base);
	ut_asserteq(gd->ram_size, info.size);

	return 0;
}
DM_TEST(dm_test_ram_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

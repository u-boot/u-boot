// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2025 Linaro Limited
 */

#include <dm.h>
#include <malloc.h>
#include <dm/test.h>
#include <asm/interconnect.h>
#include <dm/device-internal.h>
#include <dm/uclass-internal.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_interconnect(struct unit_test_state *uts)
{
	struct udevice *dev_interconnect_0,
		       *dev_interconnect_1,
		       *dev_interconnect_2,
		       *dev_interconnect_3,
		       *dev_interconnect_4;
	struct udevice *dev_test_0, *dev_test_1, *dev;
	u64 avg = 0, peak = 0;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "interconnect-test-0",
					      &dev_test_0));
	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "interconnect-test-1",
					      &dev_test_1));

	ut_assertok(sandbox_interconnect_test_get_index(dev_test_0, 0));
	ut_assertok(sandbox_interconnect_test_get(dev_test_1, "icc-path"));

	ut_assertok(uclass_find_device_by_name(UCLASS_INTERCONNECT,
					       "interconnect-0",
					      &dev_interconnect_0));
	ut_assertok(uclass_find_device_by_name(UCLASS_INTERCONNECT,
					       "interconnect-1",
					      &dev_interconnect_1));
	ut_assertok(uclass_find_device_by_name(UCLASS_INTERCONNECT,
					       "interconnect-2",
					      &dev_interconnect_2));
	ut_assertok(uclass_find_device_by_name(UCLASS_INTERCONNECT,
					       "interconnect-3",
					      &dev_interconnect_3));
	ut_assertok(uclass_find_device_by_name(UCLASS_INTERCONNECT,
					       "interconnect-4",
					      &dev_interconnect_4));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_set_bw(dev_test_0, 10000, 100000));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 10000); ut_asserteq(peak, 100000);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_set_bw(dev_test_1, 20000, 200000));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 30000); ut_asserteq(peak, 200000);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_disable(dev_test_0));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 20000); ut_asserteq(peak, 200000);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_disable(dev_test_1));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_enable(dev_test_0));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 10000); ut_asserteq(peak, 100000);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_enable(dev_test_1));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 30000); ut_asserteq(peak, 200000);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_asserteq(-EBUSY, device_remove(dev_interconnect_0, DM_REMOVE_NORMAL));
	ut_asserteq(-EBUSY, device_remove(dev_interconnect_1, DM_REMOVE_NORMAL));
	ut_asserteq(-EBUSY, device_remove(dev_interconnect_2, DM_REMOVE_NORMAL));
	ut_asserteq(-EBUSY, device_remove(dev_interconnect_3, DM_REMOVE_NORMAL));
	ut_asserteq(-EBUSY, device_remove(dev_interconnect_4, DM_REMOVE_NORMAL));

	ut_assertok(sandbox_interconnect_test_put(dev_test_0));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 20000); ut_asserteq(peak, 200000);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_assertok(sandbox_interconnect_test_put(dev_test_1));

	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_0, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_1, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_2, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_3, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);
	ut_assertok(sandbox_interconnect_get_bw(dev_interconnect_4, &avg, &peak));
	ut_asserteq(avg, 0); ut_asserteq(peak, 0);

	ut_asserteq(-ENOENT, sandbox_interconnect_test_get_index(dev_test_0, 1));
	ut_asserteq(-ENOENT, sandbox_interconnect_test_get_index(dev_test_1, 1));
	ut_asserteq(-ENODATA, sandbox_interconnect_test_get(dev_test_1, "pwet"));

	ut_assertok(device_remove(dev_interconnect_0, DM_REMOVE_NORMAL));
	ut_assertok(device_remove(dev_interconnect_1, DM_REMOVE_NORMAL));
	ut_assertok(device_remove(dev_interconnect_2, DM_REMOVE_NORMAL));
	ut_assertok(device_remove(dev_interconnect_3, DM_REMOVE_NORMAL));
	ut_assertok(device_remove(dev_interconnect_4, DM_REMOVE_NORMAL));

	ut_assertok(device_unbind(dev_interconnect_0));
	ut_assertok(device_unbind(dev_interconnect_1));
	ut_assertok(device_unbind(dev_interconnect_2));
	ut_assertok(device_unbind(dev_interconnect_3));
	ut_assertok(device_unbind(dev_interconnect_4));

	uclass_find_first_device(UCLASS_INTERCONNECT, &dev);
	ut_assert(!dev);

	uclass_find_first_device(UCLASS_ICC_NODE, &dev);
	ut_assert(!dev);

	return 0;
}

DM_TEST(dm_test_interconnect, UTF_SCAN_FDT);

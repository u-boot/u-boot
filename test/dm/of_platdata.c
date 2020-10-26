// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <test/test.h>
#include <test/ut.h>

/* Test that we can find a device using of-platdata */
static int dm_test_of_platdata_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_SERIAL, &dev));
	ut_asserteq_str("sandbox_serial", dev->name);

	return 0;
}
DM_TEST(dm_test_of_platdata_base, UT_TESTF_SCAN_PDATA);

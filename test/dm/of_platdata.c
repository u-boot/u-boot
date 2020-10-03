// SPDX-License-Identifier: GPL-2.0+

#include <common.h>
#include <dm.h>
#include <dt-structs.h>
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

/* Test that we can read properties from a device */
static int dm_test_of_platdata_props(struct unit_test_state *uts)
{
	struct dtd_sandbox_spl_test *plat;
	struct udevice *dev;
	int i;

	ut_assertok(uclass_first_device_err(UCLASS_MISC, &dev));
	plat = dev_get_platdata(dev);
	ut_assert(plat->boolval);
	ut_asserteq(1, plat->intval);
	ut_asserteq(4, ARRAY_SIZE(plat->intarray));
	ut_asserteq(2, plat->intarray[0]);
	ut_asserteq(3, plat->intarray[1]);
	ut_asserteq(4, plat->intarray[2]);
	ut_asserteq(0, plat->intarray[3]);
	ut_asserteq(5, plat->byteval);
	ut_asserteq(3, ARRAY_SIZE(plat->bytearray));
	ut_asserteq(6, plat->bytearray[0]);
	ut_asserteq(0, plat->bytearray[1]);
	ut_asserteq(0, plat->bytearray[2]);
	ut_asserteq(9, ARRAY_SIZE(plat->longbytearray));
	for (i = 0; i < ARRAY_SIZE(plat->longbytearray); i++)
		ut_asserteq(9 + i, plat->longbytearray[i]);
	ut_asserteq_str("message", plat->stringval);
	ut_asserteq(3, ARRAY_SIZE(plat->stringarray));
	ut_asserteq_str("multi-word", plat->stringarray[0]);
	ut_asserteq_str("message", plat->stringarray[1]);
	ut_asserteq_str("", plat->stringarray[2]);

	ut_assertok(uclass_next_device_err(&dev));
	plat = dev_get_platdata(dev);
	ut_assert(!plat->boolval);
	ut_asserteq(3, plat->intval);
	ut_asserteq(5, plat->intarray[0]);
	ut_asserteq(0, plat->intarray[1]);
	ut_asserteq(0, plat->intarray[2]);
	ut_asserteq(0, plat->intarray[3]);
	ut_asserteq(8, plat->byteval);
	ut_asserteq(3, ARRAY_SIZE(plat->bytearray));
	ut_asserteq(1, plat->bytearray[0]);
	ut_asserteq(0x23, plat->bytearray[1]);
	ut_asserteq(0x34, plat->bytearray[2]);
	for (i = 0; i < ARRAY_SIZE(plat->longbytearray); i++)
		ut_asserteq(i < 4 ? 9 + i : 0, plat->longbytearray[i]);
	ut_asserteq_str("message2", plat->stringval);
	ut_asserteq_str("another", plat->stringarray[0]);
	ut_asserteq_str("multi-word", plat->stringarray[1]);
	ut_asserteq_str("message", plat->stringarray[2]);

	ut_assertok(uclass_next_device_err(&dev));
	plat = dev_get_platdata(dev);
	ut_assert(!plat->boolval);
	ut_asserteq_str("one", plat->stringarray[0]);
	ut_asserteq_str("", plat->stringarray[1]);
	ut_asserteq_str("", plat->stringarray[2]);

	ut_assertok(uclass_next_device_err(&dev));
	plat = dev_get_platdata(dev);
	ut_assert(!plat->boolval);
	ut_asserteq_str("spl", plat->stringarray[0]);

	ut_asserteq(-ENODEV, uclass_next_device_err(&dev));

	return 0;
}
DM_TEST(dm_test_of_platdata_props, UT_TESTF_SCAN_PDATA);

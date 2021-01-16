// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2021 Google LLC
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

static int dm_test_cros_ec_hello(struct unit_test_state *uts)
{
	struct udevice *dev;
	uint val;

	ut_assertok(uclass_first_device_err(UCLASS_CROS_EC, &dev));

	ut_assertok(cros_ec_hello(dev, NULL));

	val = 0xdead1357;
	ut_assertok(cros_ec_hello(dev, &val));
	ut_asserteq(0xdead1357, val);

	sandbox_cros_ec_set_test_flags(dev, CROSECT_BREAK_HELLO);
	ut_asserteq(-ENOTSYNC, cros_ec_hello(dev, &val));
	ut_asserteq(0x12345678, val);

	return 0;
}
DM_TEST(dm_test_cros_ec_hello, UT_TESTF_SCAN_FDT);

static int dm_test_cros_ec_sku_id(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_CROS_EC, &dev));
	ut_asserteq(1234, cros_ec_get_sku_id(dev));

	/* try the command */
	console_record_reset();
	ut_assertok(run_command("crosec sku", 0));
	ut_assert_nextline("1234");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_cros_ec_sku_id, UT_TESTF_SCAN_FDT);

static int dm_test_cros_ec_features(struct unit_test_state *uts)
{
	struct udevice *dev;
	u64 feat;

	ut_assertok(uclass_first_device_err(UCLASS_CROS_EC, &dev));
	ut_assertok(cros_ec_get_features(dev, &feat));
	ut_asserteq_64(1U << EC_FEATURE_FLASH | 1U << EC_FEATURE_I2C |
		1ULL << EC_FEATURE_UNIFIED_WAKE_MASKS | 1ULL << EC_FEATURE_ISH,
		feat);

	ut_asserteq(true, cros_ec_check_feature(dev, EC_FEATURE_I2C));
	ut_asserteq(false, cros_ec_check_feature(dev, EC_FEATURE_MOTION_SENSE));
	ut_asserteq(true, cros_ec_check_feature(dev, EC_FEATURE_ISH));

	/* try the command */
	console_record_reset();
	ut_assertok(run_command("crosec features", 0));
	ut_assert_nextline("flash");
	ut_assert_nextline("i2c");
	ut_assert_nextline("unified_wake_masks");
	ut_assert_nextline("ish");
	ut_assert_console_end();

	return 0;
}
DM_TEST(dm_test_cros_ec_features, UT_TESTF_SCAN_FDT);

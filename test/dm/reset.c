/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <reset.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Test that we can use particular reset devices */
static int dm_test_reset_base(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct udevice *dev;

	/* Device 0 is the platform data device - it should never respond */
	ut_assertok(uclass_get_device(UCLASS_RESET, 0, &dev));
	ut_asserteq(-ENODEV, reset_request(dev, RESET_WARM));
	ut_asserteq(-ENODEV, reset_request(dev, RESET_COLD));
	ut_asserteq(-ENODEV, reset_request(dev, RESET_POWER));

	/* Device 1 is the warm reset device */
	ut_assertok(uclass_get_device(UCLASS_RESET, 1, &dev));
	ut_asserteq(-EACCES, reset_request(dev, RESET_WARM));
	ut_asserteq(-ENOSYS, reset_request(dev, RESET_COLD));
	ut_asserteq(-ENOSYS, reset_request(dev, RESET_POWER));

	state->reset_allowed[RESET_WARM] = true;
	ut_asserteq(-EINPROGRESS, reset_request(dev, RESET_WARM));
	state->reset_allowed[RESET_WARM] = false;

	/* Device 2 is the cold reset device */
	ut_assertok(uclass_get_device(UCLASS_RESET, 2, &dev));
	ut_asserteq(-ENOSYS, reset_request(dev, RESET_WARM));
	ut_asserteq(-EACCES, reset_request(dev, RESET_COLD));
	state->reset_allowed[RESET_POWER] = false;
	ut_asserteq(-EACCES, reset_request(dev, RESET_POWER));
	state->reset_allowed[RESET_POWER] = true;

	return 0;
}
DM_TEST(dm_test_reset_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can walk through the reset devices */
static int dm_test_reset_walk(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();

	/* If we generate a power reset, we will exit sandbox! */
	state->reset_allowed[RESET_POWER] = false;
	ut_asserteq(-EACCES, reset_walk(RESET_WARM));
	ut_asserteq(-EACCES, reset_walk(RESET_COLD));
	ut_asserteq(-EACCES, reset_walk(RESET_POWER));

	/*
	 * Enable cold reset - this should make cold reset work, plus a warm
	 * reset should be promoted to cold, since this is the next step
	 * along.
	 */
	state->reset_allowed[RESET_COLD] = true;
	ut_asserteq(-EINPROGRESS, reset_walk(RESET_WARM));
	ut_asserteq(-EINPROGRESS, reset_walk(RESET_COLD));
	ut_asserteq(-EACCES, reset_walk(RESET_POWER));
	state->reset_allowed[RESET_COLD] = false;
	state->reset_allowed[RESET_POWER] = true;

	return 0;
}
DM_TEST(dm_test_reset_walk, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

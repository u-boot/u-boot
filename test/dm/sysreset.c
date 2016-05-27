/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <sysreset.h>
#include <asm/state.h>
#include <asm/test.h>
#include <dm/test.h>
#include <test/ut.h>

/* Test that we can use particular sysreset devices */
static int dm_test_sysreset_base(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();
	struct udevice *dev;

	/* Device 0 is the platform data device - it should never respond */
	ut_assertok(uclass_get_device(UCLASS_SYSRESET, 0, &dev));
	ut_asserteq(-ENODEV, sysreset_request(dev, SYSRESET_WARM));
	ut_asserteq(-ENODEV, sysreset_request(dev, SYSRESET_COLD));
	ut_asserteq(-ENODEV, sysreset_request(dev, SYSRESET_POWER));

	/* Device 1 is the warm sysreset device */
	ut_assertok(uclass_get_device(UCLASS_SYSRESET, 1, &dev));
	ut_asserteq(-EACCES, sysreset_request(dev, SYSRESET_WARM));
	ut_asserteq(-ENOSYS, sysreset_request(dev, SYSRESET_COLD));
	ut_asserteq(-ENOSYS, sysreset_request(dev, SYSRESET_POWER));

	state->sysreset_allowed[SYSRESET_WARM] = true;
	ut_asserteq(-EINPROGRESS, sysreset_request(dev, SYSRESET_WARM));
	state->sysreset_allowed[SYSRESET_WARM] = false;

	/* Device 2 is the cold sysreset device */
	ut_assertok(uclass_get_device(UCLASS_SYSRESET, 2, &dev));
	ut_asserteq(-ENOSYS, sysreset_request(dev, SYSRESET_WARM));
	ut_asserteq(-EACCES, sysreset_request(dev, SYSRESET_COLD));
	state->sysreset_allowed[SYSRESET_POWER] = false;
	ut_asserteq(-EACCES, sysreset_request(dev, SYSRESET_POWER));
	state->sysreset_allowed[SYSRESET_POWER] = true;

	return 0;
}
DM_TEST(dm_test_sysreset_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can walk through the sysreset devices */
static int dm_test_sysreset_walk(struct unit_test_state *uts)
{
	struct sandbox_state *state = state_get_current();

	/* If we generate a power sysreset, we will exit sandbox! */
	state->sysreset_allowed[SYSRESET_POWER] = false;
	ut_asserteq(-EACCES, sysreset_walk(SYSRESET_WARM));
	ut_asserteq(-EACCES, sysreset_walk(SYSRESET_COLD));
	ut_asserteq(-EACCES, sysreset_walk(SYSRESET_POWER));

	/*
	 * Enable cold system reset - this should make cold system reset work,
	 * plus a warm system reset should be promoted to cold, since this is
	 * the next step along.
	 */
	state->sysreset_allowed[SYSRESET_COLD] = true;
	ut_asserteq(-EINPROGRESS, sysreset_walk(SYSRESET_WARM));
	ut_asserteq(-EINPROGRESS, sysreset_walk(SYSRESET_COLD));
	ut_asserteq(-EACCES, sysreset_walk(SYSRESET_POWER));
	state->sysreset_allowed[SYSRESET_COLD] = false;
	state->sysreset_allowed[SYSRESET_POWER] = true;

	return 0;
}
DM_TEST(dm_test_sysreset_walk, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

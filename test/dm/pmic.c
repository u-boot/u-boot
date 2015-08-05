/*
 * Tests for the driver model pmic API
 *
 * Copyright (c) 2015 Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <dm/device-internal.h>
#include <dm/root.h>
#include <dm/util.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <power/pmic.h>
#include <power/sandbox_pmic.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/* Test PMIC get method */
static int dm_test_power_pmic_get(struct unit_test_state *uts)
{
	const char *name = "sandbox_pmic";
	struct udevice *dev;

	ut_assertok(pmic_get(name, &dev));
	ut_assertnonnull(dev);

	/* Check PMIC's name */
	ut_asserteq_str(name, dev->name);

	return 0;
}
DM_TEST(dm_test_power_pmic_get, DM_TESTF_SCAN_FDT);

/* Test PMIC I/O */
static int dm_test_power_pmic_io(struct unit_test_state *uts)
{
	const char *name = "sandbox_pmic";
	uint8_t out_buffer, in_buffer;
	struct udevice *dev;
	int reg_count, i;

	ut_assertok(pmic_get(name, &dev));

	reg_count = pmic_reg_count(dev);
	ut_asserteq(reg_count, SANDBOX_PMIC_REG_COUNT);

	/*
	 * Test PMIC I/O - write and read a loop counter.
	 * usually we can't write to all PMIC's registers in the real hardware,
	 * but we can to the sandbox pmic.
	 */
	for (i = 0; i < reg_count; i++) {
		out_buffer = i;
		ut_assertok(pmic_write(dev, i, &out_buffer, 1));
		ut_assertok(pmic_read(dev, i, &in_buffer, 1));
		ut_asserteq(out_buffer, in_buffer);
	}

	return 0;
}
DM_TEST(dm_test_power_pmic_io, DM_TESTF_SCAN_FDT);

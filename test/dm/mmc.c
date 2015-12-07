/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <mmc.h>
#include <dm/test.h>
#include <test/ut.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Basic test of the mmc uclass. We could expand this by implementing an MMC
 * stack for sandbox, or at least implementing the basic operation.
 */
static int dm_test_mmc_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_get_device(UCLASS_MMC, 0, &dev));

	return 0;
}
DM_TEST(dm_test_mmc_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

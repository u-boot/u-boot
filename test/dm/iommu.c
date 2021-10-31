// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <dm/uclass-internal.h>
#include <iommu.h>
#include <test/test.h>
#include <test/ut.h>

static int dm_test_iommu(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_find_device(UCLASS_IOMMU, 0, &dev));
	ut_assert(!(dev_get_flags(dev) & DM_FLAG_ACTIVATED));

	/* Probing USB probes the IOMMU through the "iommus" property */
	ut_assertok(uclass_probe_all(UCLASS_USB));
	ut_assert(dev_get_flags(dev) & DM_FLAG_ACTIVATED);

	return 0;
}

DM_TEST(dm_test_iommu, UT_TESTF_SCAN_FDT);

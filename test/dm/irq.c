// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for irq uclass
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <dm/test.h>
#include <test/ut.h>

/* Base test of the irq uclass */
static int dm_test_irq_base(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(uclass_first_device_err(UCLASS_IRQ, &dev));

	ut_asserteq(5, irq_route_pmc_gpio_gpe(dev, 4));
	ut_asserteq(-ENOENT, irq_route_pmc_gpio_gpe(dev, 14));

	ut_assertok(irq_set_polarity(dev, 4, true));
	ut_asserteq(-EINVAL, irq_set_polarity(dev, 14, true));

	ut_assertok(irq_snapshot_polarities(dev));
	ut_assertok(irq_restore_polarities(dev));

	return 0;
}
DM_TEST(dm_test_irq_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

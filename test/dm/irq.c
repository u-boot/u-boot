// SPDX-License-Identifier: GPL-2.0+
/*
 * Test for irq uclass
 *
 * Copyright 2019 Google LLC
 */

#include <dm.h>
#include <irq.h>
#include <acpi/acpi_device.h>
#include <asm/test.h>
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
DM_TEST(dm_test_irq_base, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of irq_first_device_type() */
static int dm_test_irq_type(struct unit_test_state *uts)
{
	struct udevice *dev;

	ut_assertok(irq_first_device_type(SANDBOX_IRQT_BASE, &dev));
	ut_asserteq(-ENODEV, irq_first_device_type(X86_IRQT_BASE, &dev));

	return 0;
}
DM_TEST(dm_test_irq_type, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of irq_read_and_clear() */
static int dm_test_read_and_clear(struct unit_test_state *uts)
{
	struct irq irq;

	ut_assertok(irq_first_device_type(SANDBOX_IRQT_BASE, &irq.dev));
	irq.id = SANDBOX_IRQN_PEND;
	ut_asserteq(0, irq_read_and_clear(&irq));
	ut_asserteq(0, irq_read_and_clear(&irq));
	ut_asserteq(0, irq_read_and_clear(&irq));
	ut_asserteq(1, irq_read_and_clear(&irq));
	ut_asserteq(0, irq_read_and_clear(&irq));

	return 0;
}
DM_TEST(dm_test_read_and_clear, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of irq_request() */
static int dm_test_request(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct irq irq;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_asserteq_str("a-test", dev->name);
	ut_assertok(irq_get_by_index(dev, 0, &irq));
	ut_asserteq(3, irq.id);

	return 0;
}
DM_TEST(dm_test_request, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of irq_get_by_index() */
static int dm_test_irq_get_by_index(struct unit_test_state *uts)
{
	struct udevice *dev;
	struct irq irq;

	ut_assertok(uclass_get_device_by_name(UCLASS_TEST_FDT, "f-test",
					      &dev));
	ut_assertok(irq_get_by_index(dev, 0, &irq));
	ut_asserteq(4, irq.id);

	return 0;
}
DM_TEST(dm_test_irq_get_by_index, UTF_SCAN_PDATA | UTF_SCAN_FDT);

/* Test of irq_get_acpi() */
static int dm_test_irq_get_acpi(struct unit_test_state *uts)
{
	struct acpi_irq airq;
	struct udevice *dev;
	struct irq irq;

	ut_assertok(uclass_first_device_err(UCLASS_TEST_FDT, &dev));
	ut_assertok(irq_get_by_index(dev, 0, &irq));

	/* see sandbox_get_acpi() */
	ut_assertok(irq_get_acpi(&irq, &airq));
	ut_asserteq(3, airq.pin);
	ut_asserteq(ACPI_IRQ_LEVEL_TRIGGERED, airq.mode);
	ut_asserteq(ACPI_IRQ_ACTIVE_HIGH, airq.polarity);
	ut_asserteq(ACPI_IRQ_SHARED, airq.shared);
	ut_asserteq(ACPI_IRQ_WAKE, airq.wake);

	return 0;
}
DM_TEST(dm_test_irq_get_acpi, UTF_SCAN_PDATA | UTF_SCAN_FDT);

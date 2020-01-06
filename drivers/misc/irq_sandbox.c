// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox driver for interrupts
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <irq.h>

static int sandbox_set_polarity(struct udevice *dev, uint irq, bool active_low)
{
	if (irq > 10)
		return -EINVAL;

	return 0;
}

static int sandbox_route_pmc_gpio_gpe(struct udevice *dev, uint pmc_gpe_num)
{
	if (pmc_gpe_num > 10)
		return -ENOENT;

	return pmc_gpe_num + 1;
}

static int sandbox_snapshot_polarities(struct udevice *dev)
{
	return 0;
}

static int sandbox_restore_polarities(struct udevice *dev)
{
	return 0;
}

static const struct irq_ops sandbox_irq_ops = {
	.route_pmc_gpio_gpe	= sandbox_route_pmc_gpio_gpe,
	.set_polarity		= sandbox_set_polarity,
	.snapshot_polarities	= sandbox_snapshot_polarities,
	.restore_polarities	= sandbox_restore_polarities,
};

static const struct udevice_id sandbox_irq_ids[] = {
	{ .compatible = "sandbox,irq"},
	{ }
};

U_BOOT_DRIVER(sandbox_irq_drv) = {
	.name		= "sandbox_irq",
	.id		= UCLASS_IRQ,
	.of_match	= sandbox_irq_ids,
	.ops		= &sandbox_irq_ops,
};

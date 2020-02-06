// SPDX-License-Identifier: GPL-2.0
/*
 * Sandbox driver for interrupts
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <irq.h>
#include <asm/test.h>

/**
 * struct sandbox_irq_priv - private data for this driver
 *
 * @count: Counts the number calls to the read_and_clear() method
 * @pending: true if an interrupt is pending, else false
 */
struct sandbox_irq_priv {
	int count;
	bool pending;
};

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

static int sandbox_irq_read_and_clear(struct irq *irq)
{
	struct sandbox_irq_priv *priv = dev_get_priv(irq->dev);

	if (irq->id != SANDBOX_IRQN_PEND)
		return -EINVAL;
	priv->count++;
	if (priv->pending) {
		priv->pending = false;
		return 1;
	}

	if (!(priv->count % 3))
		priv->pending = true;

	return 0;
}

static int sandbox_irq_of_xlate(struct irq *irq,
				struct ofnode_phandle_args *args)
{
	irq->id = args->args[0];

	return 0;
}

static const struct irq_ops sandbox_irq_ops = {
	.route_pmc_gpio_gpe	= sandbox_route_pmc_gpio_gpe,
	.set_polarity		= sandbox_set_polarity,
	.snapshot_polarities	= sandbox_snapshot_polarities,
	.restore_polarities	= sandbox_restore_polarities,
	.read_and_clear		= sandbox_irq_read_and_clear,
	.of_xlate		= sandbox_irq_of_xlate,
};

static const struct udevice_id sandbox_irq_ids[] = {
	{ .compatible = "sandbox,irq", SANDBOX_IRQT_BASE },
	{ }
};

U_BOOT_DRIVER(sandbox_irq_drv) = {
	.name		= "sandbox_irq",
	.id		= UCLASS_IRQ,
	.of_match	= sandbox_irq_ids,
	.ops		= &sandbox_irq_ops,
	.priv_auto_alloc_size	= sizeof(struct sandbox_irq_priv),
};

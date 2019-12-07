// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <irq.h>

int irq_route_pmc_gpio_gpe(struct udevice *dev, uint pmc_gpe_num)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->route_pmc_gpio_gpe)
		return -ENOSYS;

	return ops->route_pmc_gpio_gpe(dev, pmc_gpe_num);
}

int irq_set_polarity(struct udevice *dev, uint irq, bool active_low)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->set_polarity)
		return -ENOSYS;

	return ops->set_polarity(dev, irq, active_low);
}

int irq_snapshot_polarities(struct udevice *dev)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->snapshot_polarities)
		return -ENOSYS;

	return ops->snapshot_polarities(dev);
}

int irq_restore_polarities(struct udevice *dev)
{
	const struct irq_ops *ops = irq_get_ops(dev);

	if (!ops->restore_polarities)
		return -ENOSYS;

	return ops->restore_polarities(dev);
}

UCLASS_DRIVER(irq) = {
	.id		= UCLASS_IRQ,
	.name		= "irq",
};

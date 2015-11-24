/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Implement a timer uclass to work with lib/time.c. The timer is usually
 * a 32/64 bits free-running up counter. The get_rate() method is used to get
 * the input clock frequency of the timer. The get_count() method is used
 * to get the current 64 bits count value. If the hardware is counting down,
 * the value should be inversed inside the method. There may be no real
 * tick, and no timer interrupt.
 */

int timer_get_count(struct udevice *dev, u64 *count)
{
	const struct timer_ops *ops = device_get_ops(dev);

	if (!ops->get_count)
		return -ENOSYS;

	return ops->get_count(dev, count);
}

unsigned long timer_get_rate(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	return uc_priv->clock_rate;
}

static int timer_pre_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->clock_rate = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					     "clock-frequency", 0);

	return 0;
}

u64 timer_conv_64(u32 count)
{
	/* increment tbh if tbl has rolled over */
	if (count < gd->timebase_l)
		gd->timebase_h++;
	gd->timebase_l = count;
	return ((u64)gd->timebase_h << 32) | gd->timebase_l;
}

UCLASS_DRIVER(timer) = {
	.id		= UCLASS_TIMER,
	.name		= "timer",
	.pre_probe	= timer_pre_probe,
	.per_device_auto_alloc_size = sizeof(struct timer_dev_priv),
};

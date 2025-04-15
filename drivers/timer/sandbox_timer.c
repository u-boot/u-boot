// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 */

#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <os.h>

#define SANDBOX_TIMER_RATE	1000000

/* system timer offset in ms */
static unsigned long sandbox_timer_offset;

void timer_test_add_offset(unsigned long offset)
{
	sandbox_timer_offset += offset;
}

ulong timer_test_get_offset(void)
{
	return sandbox_timer_offset;
};

u64 notrace timer_early_get_count(void)
{
	return os_get_nsec() / 1000 + sandbox_timer_offset * 1000;
}

unsigned long notrace timer_early_get_rate(void)
{
	return SANDBOX_TIMER_RATE;
}

static notrace u64 sandbox_timer_get_count(struct udevice *dev)
{
	return timer_early_get_count();
}

static int sandbox_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (CONFIG_IS_ENABLED(CPU) &&
	    dev_read_bool(dev, "sandbox,timebase-frequency-fallback"))
		return timer_timebase_fallback(dev);
	else if (!uc_priv->clock_rate)
		uc_priv->clock_rate = SANDBOX_TIMER_RATE;

	return 0;
}

static const struct timer_ops sandbox_timer_ops = {
	.get_count = sandbox_timer_get_count,
};

static const struct udevice_id sandbox_timer_ids[] = {
	{ .compatible = "sandbox,timer" },
	{ }
};

U_BOOT_DRIVER(sandbox_timer) = {
	.name	= "sandbox_timer",
	.id	= UCLASS_TIMER,
	.of_match = sandbox_timer_ids,
	.probe = sandbox_timer_probe,
	.ops	= &sandbox_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/* This is here in case we don't have a device tree */
#if !CONFIG_IS_ENABLED(OF_PLATDATA)
U_BOOT_DRVINFO(sandbox_timer_non_fdt) = {
	.name = "sandbox_timer",
};
#endif

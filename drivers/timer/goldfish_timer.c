// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * Goldfish Timer driver
 */

#include <dm.h>
#include <goldfish_timer.h>
#include <mapmem.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/errno.h>

struct goldfish_timer_priv {
	void __iomem *base;
};

/* Goldfish RTC registers used as Timer */
#define TIMER_TIME_LOW	0x00
#define TIMER_TIME_HIGH	0x04

static u64 goldfish_timer_get_count(struct udevice *dev)
{
	struct goldfish_timer_priv *priv = dev_get_priv(dev);
	u32 low, high;
	u64 time;

	/*
	 * TIMER_TIME_HIGH is only updated when TIMER_TIME_LOW is read.
	 * We must read LOW before HIGH to latch the high 32-bit value
	 * and ensure a consistent 64-bit timestamp.
	 */
	low = readl(priv->base + TIMER_TIME_LOW);
	high = readl(priv->base + TIMER_TIME_HIGH);

	time = ((u64)high << 32) | low;

	return time;
}

static int goldfish_timer_of_to_plat(struct udevice *dev)
{
	struct goldfish_timer_plat *plat = dev_get_plat(dev);
	fdt_addr_t addr;

	addr = dev_read_addr(dev);
	if (addr != FDT_ADDR_T_NONE)
		plat->reg = addr;

	return 0;
}

static int goldfish_timer_probe(struct udevice *dev)
{
	struct goldfish_timer_plat *plat = dev_get_plat(dev);
	struct goldfish_timer_priv *priv = dev_get_priv(dev);
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	if (!plat->reg)
		return -EINVAL;

	priv->base = map_sysmem(plat->reg, 0x20);

	/* Goldfish RTC counts in nanoseconds, so the rate is 1GHz */
	uc_priv->clock_rate = 1000000000;

	return 0;
}

static const struct timer_ops goldfish_timer_ops = {
	.get_count = goldfish_timer_get_count,
};

static const struct udevice_id goldfish_timer_ids[] = {
	{ .compatible = "google,goldfish-rtc" },
	{ }
};

U_BOOT_DRIVER(goldfish_timer) = {
	.name	= "goldfish_timer",
	.id	= UCLASS_TIMER,
	.of_match = goldfish_timer_ids,
	.of_to_plat = goldfish_timer_of_to_plat,
	.plat_auto  = sizeof(struct goldfish_timer_plat),
	.ops	= &goldfish_timer_ops,
	.probe	= goldfish_timer_probe,
	.priv_auto = sizeof(struct goldfish_timer_priv),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <linux/err.h>

/* mtime register */
#define MTIME_REG(base)			((ulong)(base) + 0xbff8)

static u64 sifive_clint_get_count(struct udevice *dev)
{
	return readq((void __iomem *)MTIME_REG(dev->priv));
}

static const struct timer_ops sifive_clint_ops = {
	.get_count = sifive_clint_get_count,
};

static int sifive_clint_probe(struct udevice *dev)
{
	dev->priv = dev_read_addr_ptr(dev);
	if (!dev->priv)
		return -EINVAL;

	return timer_timebase_fallback(dev);
}

static const struct udevice_id sifive_clint_ids[] = {
	{ .compatible = "riscv,clint0" },
	{ }
};

U_BOOT_DRIVER(sifive_clint) = {
	.name		= "sifive_clint",
	.id		= UCLASS_TIMER,
	.of_match	= sifive_clint_ids,
	.probe		= sifive_clint_probe,
	.ops		= &sifive_clint_ops,
	.flags		= DM_FLAG_PRE_RELOC,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * U-Boot syscon driver for SiFive's Core Local Interruptor (CLINT).
 * The CLINT block holds memory-mapped control and status registers
 * associated with software and timer interrupts.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <timer.h>
#include <asm/io.h>
#include <asm/syscon.h>
#include <linux/err.h>

/* MSIP registers */
#define MSIP_REG(base, hart)		((ulong)(base) + (hart) * 4)
/* mtime compare register */
#define MTIMECMP_REG(base, hart)	((ulong)(base) + 0x4000 + (hart) * 8)
/* mtime register */
#define MTIME_REG(base)			((ulong)(base) + 0xbff8)

DECLARE_GLOBAL_DATA_PTR;

int riscv_init_ipi(void)
{
	int ret;
	struct udevice *dev;

	ret = uclass_get_device_by_driver(UCLASS_TIMER,
					  DM_GET_DRIVER(sifive_clint), &dev);
	if (ret)
		return ret;

	gd->arch.clint = dev_read_addr_ptr(dev);
	if (!gd->arch.clint)
		return -EINVAL;

	return 0;
}

int riscv_send_ipi(int hart)
{
	writel(1, (void __iomem *)MSIP_REG(gd->arch.clint, hart));

	return 0;
}

int riscv_clear_ipi(int hart)
{
	writel(0, (void __iomem *)MSIP_REG(gd->arch.clint, hart));

	return 0;
}

int riscv_get_ipi(int hart, int *pending)
{
	*pending = readl((void __iomem *)MSIP_REG(gd->arch.clint, hart));

	return 0;
}

static int sifive_clint_get_count(struct udevice *dev, u64 *count)
{
	*count = readq((void __iomem *)MTIME_REG(dev->priv));

	return 0;
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

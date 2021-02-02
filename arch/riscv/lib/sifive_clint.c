// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * U-Boot syscon driver for SiFive's Core Local Interruptor (CLINT).
 * The CLINT block holds memory-mapped control and status registers
 * associated with software and timer interrupts.
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/smp.h>
#include <linux/err.h>

/* MSIP registers */
#define MSIP_REG(base, hart)		((ulong)(base) + (hart) * 4)

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

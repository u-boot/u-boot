// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Sean Anderson <seanga2@gmail.com>
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 * Copyright (C) 2018, Anup Patel <anup@brainfault.org>
 * Copyright (C) 2012 Regents of the University of California
 *
 * RISC-V architecturally-defined generic timer driver
 *
 * This driver provides generic timer support for S-mode U-Boot.
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <timer.h>
#include <asm/csr.h>

static u64 riscv_timer_get_count(struct udevice *dev)
{
	__maybe_unused u32 hi, lo;

	if (IS_ENABLED(CONFIG_64BIT))
		return csr_read(CSR_TIME);

	do {
		hi = csr_read(CSR_TIMEH);
		lo = csr_read(CSR_TIME);
	} while (hi != csr_read(CSR_TIMEH));

	return ((u64)hi << 32) | lo;
}

static int riscv_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	/* clock frequency was passed from the cpu driver as driver data */
	uc_priv->clock_rate = dev->driver_data;

	return 0;
}

static const struct timer_ops riscv_timer_ops = {
	.get_count = riscv_timer_get_count,
};

U_BOOT_DRIVER(riscv_timer) = {
	.name = "riscv_timer",
	.id = UCLASS_TIMER,
	.probe = riscv_timer_probe,
	.ops = &riscv_timer_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/ioapic.h>

u32 io_apic_read(u32 reg)
{
	writel(reg, IO_APIC_INDEX);
	return readl(IO_APIC_DATA);
}

void io_apic_write(u32 reg, u32 val)
{
	writel(reg, IO_APIC_INDEX);
	writel(val, IO_APIC_DATA);
}

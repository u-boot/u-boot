// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <asm/arch/sys_proto.h>

u32 get_cpu_rev(void)
{
	return (MXC_CPU_IMX8ULP << 12) | CHIP_REV_1_0;
}

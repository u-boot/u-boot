/*
 * arch/arm/cpu/armv7/rmobile/cpu_info-rcar.c
 *
 * Copyright (C) 2013,2014 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
 */
#include <common.h>
#include <asm/io.h>

#define PRR			0xFF000044
#define PRR_MASK		0x7fff
#define R8A7796_REV_1_0		0x5200
#define R8A7796_REV_1_1		0x5210

u32 rmobile_get_cpu_type(void)
{
	return (readl(PRR) & 0x00007F00) >> 8;
}

u32 rmobile_get_cpu_rev_integer(void)
{
	const u32 prr = readl(PRR);

	if ((prr & PRR_MASK) == R8A7796_REV_1_1)
		return 1;
	else
		return ((prr & 0x000000F0) >> 4) + 1;
}

u32 rmobile_get_cpu_rev_fraction(void)
{
	const u32 prr = readl(PRR);

	if ((prr & PRR_MASK) == R8A7796_REV_1_1)
		return 1;
	else
		return prr & 0x0000000F;
}

/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <mach/sbc-regs.h>
#include <mach/sg-regs.h>

void sbc_init(void)
{
	u32 tmp;

	/* system bus output enable */
	tmp = readl(PC0CTRL);
	tmp &= 0xfffffcff;
	writel(tmp, PC0CTRL);

	/* XECS1: sub/boot memory (boot swap = off/on) */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL10);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL11);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL12);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL14);

	/* XECS0: boot/sub memory (boot swap = off/on) */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL00);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL01);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL02);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL04);

	/* XECS3: peripherals */
	writel(SBCTRL0_SAVEPIN_PERI_VALUE, SBCTRL30);
	writel(SBCTRL1_SAVEPIN_PERI_VALUE, SBCTRL31);
	writel(SBCTRL2_SAVEPIN_PERI_VALUE, SBCTRL32);
	writel(SBCTRL4_SAVEPIN_PERI_VALUE, SBCTRL34);

	/* base address regsiters */
	writel(0x0000bc01, SBBASE0);
	writel(0x0400bc01, SBBASE1);
	writel(0x0800bf01, SBBASE3);

	/* enable access to sub memory when boot swap is on */
	if (boot_is_swapped())
		sg_set_pinsel(155, 1); /* PORT24 -> XECS0 */

	sg_set_pinsel(156, 1); /* PORT25 -> XECS3 */
}

/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <mach/sbc-regs.h>
#include <mach/sg-regs.h>

void sbc_init(void)
{
	/* XECS0: boot/sub memory (boot swap = off/on) */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL00);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL01);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL02);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL04);

	/* XECS1: sub/boot memory (boot swap = off/on) */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL10);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL11);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL12);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL14);

	/* XECS3: peripherals */
	writel(SBCTRL0_SAVEPIN_PERI_VALUE, SBCTRL30);
	writel(SBCTRL1_SAVEPIN_PERI_VALUE, SBCTRL31);
	writel(SBCTRL2_SAVEPIN_PERI_VALUE, SBCTRL32);
	writel(SBCTRL4_SAVEPIN_PERI_VALUE, SBCTRL34);

	writel(0x0000bc01, SBBASE0); /* boot memory */
	writel(0x0400bc01, SBBASE1); /* sub memory */
	writel(0x0800bf01, SBBASE3); /* peripherals */

	/* enable access to sub memory when boot swap is on */
	if (boot_is_swapped())
		sg_set_pinsel(318, 5); /* PORT22 -> XECS0 */

	sg_set_pinsel(313, 5); /* PORT15 -> XECS3 */
	writel(0x00000001, SG_LOADPINCTRL);
}

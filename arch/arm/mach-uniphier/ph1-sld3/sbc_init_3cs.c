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
	/* only address/data multiplex mode is supported */

	/* XECS0 : boot/sub memory (boot swap = off/on) */
	writel(SBCTRL0_ADMULTIPLX_MEM_VALUE, SBCTRL00);
	writel(SBCTRL1_ADMULTIPLX_MEM_VALUE, SBCTRL01);
	writel(SBCTRL2_ADMULTIPLX_MEM_VALUE, SBCTRL02);

	/* XECS1 : sub/boot memory (boot swap = off/on) */
	writel(SBCTRL0_ADMULTIPLX_MEM_VALUE, SBCTRL10);
	writel(SBCTRL1_ADMULTIPLX_MEM_VALUE, SBCTRL11);
	writel(SBCTRL2_ADMULTIPLX_MEM_VALUE, SBCTRL12);

	/* XECS2 : peripherals */
	writel(SBCTRL0_ADMULTIPLX_PERI_VALUE, SBCTRL20);
	writel(SBCTRL1_ADMULTIPLX_PERI_VALUE, SBCTRL21);
	writel(SBCTRL2_ADMULTIPLX_PERI_VALUE, SBCTRL22);

	/* base address regsiters */
	writel(0x0000bc01, SBBASE0);
	writel(0x0400bc01, SBBASE1);
	writel(0x0800bf01, SBBASE2);

	sg_set_pinsel(99, 1);	/* GPIO26 -> EA24 */
}

/*
 * Copyright (C) 2011-2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sbc-regs.h>
#include <asm/arch/sg-regs.h>

void sbc_init(void)
{
#if !defined(CONFIG_SPL_BUILD)
	/* XECS0 : dummy */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL00);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL01);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL02);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL04);
#endif
	/* XECS1 : boot memory (always boot swap = on) */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL10);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL11);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL12);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL14);

	/* XECS4 : sub memory */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL40);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL41);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL42);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL44);

	/* XECS5 : peripherals */
	writel(SBCTRL0_SAVEPIN_PERI_VALUE, SBCTRL50);
	writel(SBCTRL1_SAVEPIN_PERI_VALUE, SBCTRL51);
	writel(SBCTRL2_SAVEPIN_PERI_VALUE, SBCTRL52);
	writel(SBCTRL4_SAVEPIN_PERI_VALUE, SBCTRL54);

	/* base address regsiters */
	writel(0x0000bc01, SBBASE0); /* boot memory */
	writel(0x0900bfff, SBBASE1); /* dummy */
	writel(0x0400bc01, SBBASE4); /* sub memory */
	writel(0x0800bf01, SBBASE5); /* peripherals */

	sg_set_pinsel(134, 16); /* XIRQ6 -> XECS4 */
	sg_set_pinsel(135, 16); /* XIRQ7 -> XECS5 */

	/* dummy read to assure write process */
	readl(SG_PINCTRL(33));
}

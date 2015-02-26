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
#if defined(CONFIG_PFC_MICRO_SUPPORT_CARD)
	/*
	 * Only CS1 is connected to support card.
	 * BKSZ[1:0] should be set to "01".
	 */
	writel(SBCTRL0_SAVEPIN_PERI_VALUE, SBCTRL10);
	writel(SBCTRL1_SAVEPIN_PERI_VALUE, SBCTRL11);
	writel(SBCTRL2_SAVEPIN_PERI_VALUE, SBCTRL12);
	writel(SBCTRL4_SAVEPIN_PERI_VALUE, SBCTRL14);

	if (boot_is_swapped()) {
		/*
		 * Boot Swap On: boot from external NOR/SRAM
		 * 0x02000000-0x03ffffff is a mirror of 0x00000000-0x01ffffff.
		 *
		 * 0x00000000-0x01efffff, 0x02000000-0x03efffff: memory bank
		 * 0x01f00000-0x01ffffff, 0x03f00000-0x03ffffff: peripherals
		 */
		writel(0x0000bc01, SBBASE0);
	} else {
		/*
		 * Boot Swap Off: boot from mask ROM
		 * 0x00000000-0x01ffffff: mask ROM
		 * 0x02000000-0x3effffff: memory bank (31MB)
		 * 0x03f00000-0x3fffffff: peripherals (1MB)
		 */
		writel(0x0000be01, SBBASE0); /* dummy */
		writel(0x0200be01, SBBASE1);
	}
#elif defined(CONFIG_DCC_MICRO_SUPPORT_CARD)
#if !defined(CONFIG_SPL_BUILD)
	/* XECS0: boot/sub memory (boot swap = off/on) */
	writel(SBCTRL0_SAVEPIN_MEM_VALUE, SBCTRL00);
	writel(SBCTRL1_SAVEPIN_MEM_VALUE, SBCTRL01);
	writel(SBCTRL2_SAVEPIN_MEM_VALUE, SBCTRL02);
	writel(SBCTRL4_SAVEPIN_MEM_VALUE, SBCTRL04);
#endif
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

#if !defined(CONFIG_SPL_BUILD)
	sg_set_pinsel(318, 5); /* PORT22 -> XECS0 */
#endif
	sg_set_pinsel(313, 5); /* PORT15 -> XECS3 */
	writel(0x00000001, SG_LOADPINCTRL);

#endif /* CONFIG_XXX_MICRO_SUPPORT_CARD */
}

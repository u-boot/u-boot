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
		 * 0x02000000-0x03efffff: memory bank (31MB)
		 * 0x03f00000-0x03ffffff: peripherals (1MB)
		 */
		writel(0x0000be01, SBBASE0); /* dummy */
		writel(0x0200be01, SBBASE1);
	}
}

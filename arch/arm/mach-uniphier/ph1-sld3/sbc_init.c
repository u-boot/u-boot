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

	/*
	 * Only CS1 is connected to support card.
	 * BKSZ[1:0] should be set to "01".
	 */
	writel(SBCTRL0_ADMULTIPLX_MEM_VALUE, SBCTRL10);
	writel(SBCTRL1_ADMULTIPLX_MEM_VALUE, SBCTRL11);
	writel(SBCTRL2_ADMULTIPLX_MEM_VALUE, SBCTRL12);

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

	sg_set_pinsel(99, 1);	/* GPIO26 -> EA24 */
}

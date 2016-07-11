/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"
#include "sbc-regs.h"

#define SBCTRL0_ADMULTIPLX_PERI_VALUE	0x33120000
#define SBCTRL1_ADMULTIPLX_PERI_VALUE	0x03005500
#define SBCTRL2_ADMULTIPLX_PERI_VALUE	0x14000020

#define SBCTRL0_ADMULTIPLX_MEM_VALUE	0x33120000
#define SBCTRL1_ADMULTIPLX_MEM_VALUE	0x03005500
#define SBCTRL2_ADMULTIPLX_MEM_VALUE	0x14000010

int uniphier_sbc_init_admulti(const struct uniphier_board_data *bd)
{
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
		 * 0x42000000-0x43ffffff is a mirror of 0x40000000-0x41ffffff.
		 *
		 * 0x40000000-0x41efffff, 0x42000000-0x43efffff: memory bank
		 * 0x41f00000-0x41ffffff, 0x43f00000-0x43ffffff: peripherals
		 */
		writel(0x0000bc01, SBBASE0);
	} else {
		/*
		 * Boot Swap Off: boot from mask ROM
		 * 0x40000000-0x41ffffff: mask ROM
		 * 0x42000000-0x43efffff: memory bank (31MB)
		 * 0x43f00000-0x43ffffff: peripherals (1MB)
		 */
		writel(0x0000be01, SBBASE0); /* dummy */
		writel(0x0200be01, SBBASE1);
	}

	return 0;
}

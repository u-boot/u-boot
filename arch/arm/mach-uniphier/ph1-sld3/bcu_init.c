/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/io.h>
#include <mach/bcu-regs.h>

#define ch(x) ((x) >= 32 ? 0 : (x) < 0 ? 0x11111111 : 0x11111111 << (x))

void bcu_init(void)
{
	int shift;

	writel(0x11111111, BCSCR2); /* 0x80000000-0x9fffffff: IPPC/IPPD-bus */
	writel(0x11111111, BCSCR3); /* 0xa0000000-0xbfffffff: IPPC/IPPD-bus */
	writel(0x11111111, BCSCR4); /* 0xc0000000-0xdfffffff: IPPC/IPPD-bus */
	/*
	 * 0xe0000000-0xefffffff: Ex-bus
	 * 0xf0000000-0xfbffffff: ASM bus
	 * 0xfc000000-0xffffffff: OCM bus
	 */
	writel(0x24440000, BCSCR5);

	/* Specify DDR channel */
	shift = (CONFIG_SDRAM1_BASE - CONFIG_SDRAM0_BASE) / 0x04000000 * 4;
	writel(ch(shift), BCIPPCCHR2); /* 0x80000000-0x9fffffff */

	shift -= 32;
	writel(ch(shift), BCIPPCCHR3); /* 0xa0000000-0xbfffffff */

	shift -= 32;
	writel(ch(shift), BCIPPCCHR4); /* 0xc0000000-0xdfffffff */
}

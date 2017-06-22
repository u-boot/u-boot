/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>
#include <linux/sizes.h>

#include "../init.h"

#define UNIPHIER_SMPCTRL_ROM_RSV0	0x59801200

void uniphier_smp_setup(void);
void uniphier_secondary_startup(void);

void uniphier_smp_kick_all_cpus(void)
{
	void __iomem *rom_boot_rsv0;

	rom_boot_rsv0 = ioremap(UNIPHIER_SMPCTRL_ROM_RSV0, SZ_8);

	writeq((u64)uniphier_secondary_startup, rom_boot_rsv0);

	iounmap(rom_boot_rsv0);

	uniphier_smp_setup();

	asm("dsb	ishst\n" /* Ensure the write to ROM_RSV0 is visible */
	    "sev"); /* Bring up all secondary CPUs from Boot ROM into U-Boot */
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/falcon/falcon.c
 *     This file is Falcon board support.
 *
 * Copyright (C) 2020 Renesas Electronics Corp.
 */

#include <common.h>
#include <asm/arch/rmobile.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <linux/errno.h>

DECLARE_GLOBAL_DATA_PTR;

#define CPGWPR		0xE6150000
#define CPGWPCR		0xE6150004

int board_early_init_f(void)
{
	/* Unlock CPG access */
	writel(0x5A5AFFFF, CPGWPR);
	writel(0xA5A50000, CPGWPCR);

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_SRESCR0	(RST_BASE + 0x18)
#define RST_SPRES	0x5AA58000

void reset_cpu(void)
{
	writel(RST_SPRES, RST_SRESCR0);
}

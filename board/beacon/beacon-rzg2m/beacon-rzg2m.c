// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 Compass Electronics Group, LLC
 */

#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/rcar-mstp.h>

DECLARE_GLOBAL_DATA_PTR;

void s_init(void)
{
}

/* Kconfig forces this on, so just return 0 */
int board_early_init_f(void)
{
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_TEXT_BASE + 0x50000;

	return 0;
}

#define RST_BASE	0xE6160000
#define RST_CA57RESCNT	(RST_BASE + 0x40)
#define RST_CODE	0xA5A5000F

void reset_cpu(void)
{
	writel(RST_CODE, RST_CA57RESCNT);
}

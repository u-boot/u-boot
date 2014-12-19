/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/boot-device.h>
#include <asm/arch/sbc-regs.h>

static int do_pinmon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct boot_device_info *table;
	u32 mode_sel, n = 0;

	mode_sel = get_boot_mode_sel();

	printf("Boot Swap: %s\n\n", boot_is_swapped() ? "ON" : "OFF");

	puts("Boot Mode Pin:\n");

	for (table = boot_device_table; strlen(table->info); table++) {
		printf(" %c %02x %s\n", n == mode_sel ? '*' : ' ', n,
		       table->info);
		n++;
	}

	return 0;
}

U_BOOT_CMD(
	pinmon,	1,	1,	do_pinmon,
	"pin monitor",
	""
);

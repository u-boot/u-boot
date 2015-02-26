/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mach/boot-device.h>
#include <mach/sbc-regs.h>

static int do_pinmon(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int mode_sel, i;

	printf("Boot Swap: %s\n\n", boot_is_swapped() ? "ON" : "OFF");

	mode_sel = get_boot_mode_sel();

	puts("Boot Mode Pin:\n");

	for (i = 0; boot_device_table[i].info; i++)
		printf(" %c %02x %s\n", i == mode_sel ? '*' : ' ', i,
		       boot_device_table[i].info);

	return 0;
}

U_BOOT_CMD(
	pinmon,	1,	1,	do_pinmon,
	"pin monitor",
	""
);

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <command.h>
#include <display_options.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static int do_meminfo(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	puts("DRAM:  ");
	print_size(gd->ram_size, "\n");

	return 0;
}

U_BOOT_CMD(
	meminfo,	1,	1,	do_meminfo,
	"display memory information",
	""
);

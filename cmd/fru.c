// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2019 Xilinx, Inc.
 */

#include <common.h>
#include <fdtdec.h>
#include <malloc.h>
#include <fru.h>

static int do_fru_capture(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	unsigned long addr;
	char *endp;

	if (argc < cmdtp->maxargs)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[2], &endp, 16);
	if (*argv[1] == 0 || *endp != 0)
		return -1;

	return fru_capture(addr);
}

static int do_fru_display(cmd_tbl_t *cmdtp, int flag, int argc,
			  char *const argv[])
{
	fru_display();
	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_fru_sub[] = {
	U_BOOT_CMD_MKENT(capture, 3, 0, do_fru_capture, "", ""),
	U_BOOT_CMD_MKENT(display, 2, 0, do_fru_display, "", ""),
};

static int do_fru(cmd_tbl_t *cmdtp, int flag, int argc,
		  char *const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	c = find_cmd_tbl(argv[1], &cmd_fru_sub[0],
			 ARRAY_SIZE(cmd_fru_sub));
	if (c)
		return c->cmd(c, flag, argc, argv);

	return CMD_RET_USAGE;
}

/***************************************************/
#ifdef CONFIG_SYS_LONGHELP
static char fru_help_text[] =
	"capture <addr> - Parse and capture FRU table present at address.\n"
	"fru display - Displays content of FRU table that was captured using\n"
	"              fru capture command\n"
	;
#endif

U_BOOT_CMD(
	fru, 3, 1, do_fru,
	"FRU table info",
	fru_help_text
)

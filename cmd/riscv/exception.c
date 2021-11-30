// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'exception' command can be used for testing exception handling.
 *
 * Copyright (c) 2018, Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <common.h>
#include <command.h>

static int do_ebreak(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	asm volatile ("ebreak\n");
	return CMD_RET_FAILURE;
}

static int do_unaligned(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	asm volatile (
		"auipc a1, 0\n"
		"ori   a1, a1, 3\n"
		"lw    a2, (0)(a1)\n"
	);
	printf("The system supports unaligned access.\n");
	return CMD_RET_SUCCESS;
}

static int do_undefined(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	asm volatile (".word 0xffffffff\n");
	return CMD_RET_FAILURE;
}

static struct cmd_tbl cmd_sub[] = {
	U_BOOT_CMD_MKENT(ebreak, CONFIG_SYS_MAXARGS, 1, do_ebreak,
			 "", ""),
	U_BOOT_CMD_MKENT(unaligned, CONFIG_SYS_MAXARGS, 1, do_unaligned,
			 "", ""),
	U_BOOT_CMD_MKENT(undefined, CONFIG_SYS_MAXARGS, 1, do_undefined,
			 "", ""),
};

static char exception_help_text[] =
	"<ex>\n"
	"  The following exceptions are available:\n"
	"  ebreak    - breakpoint\n"
	"  undefined - illegal instruction\n"
	"  unaligned - load address misaligned\n"
	;

#include <exception.h>

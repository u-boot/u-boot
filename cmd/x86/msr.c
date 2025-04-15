// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'cpuid' command provides access to the CPU's cpuid information
 *
 * Copyright 2024 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <command.h>
#include <vsprintf.h>
#include <asm/msr.h>

static int do_read(struct cmd_tbl *cmdtp, int flag, int argc,
		   char *const argv[])
{
	struct msr_t msr;
	ulong op;

	if (argc < 2)
		return CMD_RET_USAGE;

	op = hextoul(argv[1], NULL);
	msr = msr_read(op);
	printf("%08x %08x\n", msr.hi, msr.lo);

	return 0;
}

static int do_write(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct msr_t msr;
	ulong op;

	if (argc < 4)
		return CMD_RET_USAGE;

	op = hextoul(argv[1], NULL);
	msr.hi = hextoul(argv[2], NULL);
	msr.lo = hextoul(argv[3], NULL);
	msr_write(op, msr);

	return 0;
}

U_BOOT_LONGHELP(msr,
	"read <op>         - read a machine-status register (MSR) as <hi 32-bits> <lo 32-bits>\n"
	"write <op< <hi> <lo>  - write an MSR");

U_BOOT_CMD_WITH_SUBCMDS(msr, "Machine Status Registers", msr_help_text,
	U_BOOT_CMD_MKENT(read, CONFIG_SYS_MAXARGS, 1, do_read, "", ""),
	U_BOOT_CMD_MKENT(write, CONFIG_SYS_MAXARGS, 1, do_write, "", ""));

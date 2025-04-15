// SPDX-License-Identifier: GPL-2.0+
/*
 * The 'cpuid' command provides access to the CPU's cpuid information
 *
 * Copyright 2024 Google, LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <command.h>
#include <vsprintf.h>
#include <asm/cpu.h>

static int do_cpuid(struct cmd_tbl *cmdtp, int flag, int argc,
		    char *const argv[])
{
	struct cpuid_result res;
	ulong op;

	if (argc < 2)
		return CMD_RET_USAGE;

	op = hextoul(argv[1], NULL);
	res = cpuid(op);
	printf("eax %08x\n", res.eax);
	printf("ebx %08x\n", res.ebx);
	printf("ecx %08x\n", res.ecx);
	printf("edx %08x\n", res.edx);

	return 0;
}

U_BOOT_LONGHELP(cpuid, "Show CPU Identification information");

U_BOOT_CMD(
	cpuid,	2,	1,	do_cpuid,
	"cpuid <op>", cpuid_help_text
);

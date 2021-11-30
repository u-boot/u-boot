// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 */

#include <common.h>
#include <command.h>
#include <init.h>
#include <linux/compiler.h>
#include <asm/cache.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>

#if !CONFIG_IS_ENABLED(SYSRESET)
void __weak _machine_restart(void)
{
	fprintf(stderr, "*** reset failed ***\n");

	while (1)
		/* NOP */;
}

int do_reset(struct cmd_tbl *cmdtp, int flag, int argc, char *const argv[])
{
	_machine_restart();

	return 0;
}
#endif

int arch_cpu_init(void)
{
	mips_cache_probe();
	return 0;
}

/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <linux/compiler.h>
#include <asm/cache.h>
#include <asm/mipsregs.h>
#include <asm/reboot.h>

void __weak _machine_restart(void)
{
	fprintf(stderr, "*** reset failed ***\n");

	while (1)
		/* NOP */;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	_machine_restart();

	return 0;
}

void write_one_tlb(int index, u32 pagemask, u32 hi, u32 low0, u32 low1)
{
	write_c0_entrylo0(low0);
	write_c0_pagemask(pagemask);
	write_c0_entrylo1(low1);
	write_c0_entryhi(hi);
	write_c0_index(index);
	tlb_write_indexed();
}

int arch_cpu_init(void)
{
	mips_cache_probe();
	return 0;
}

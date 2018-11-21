// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2007
 * Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * (C) Copyright 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 */

#include <common.h>
#include <command.h>
#include <asm/processor.h>

int checkcpu(void)
{
	puts("CPU: SH3\n");
	return 0;
}

int cpu_init(void)
{
	return 0;
}

int cleanup_before_linux(void)
{
	disable_interrupts();
	return 0;
}

int do_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	reset_cpu(0);
	return 0;
}

void flush_cache(unsigned long addr, unsigned long size)
{

}

void icache_enable(void)
{
}

void icache_disable(void)
{
}

int icache_status(void)
{
	return 0;
}

void dcache_enable(void)
{
}

void dcache_disable(void)
{
}

int dcache_status(void)
{
	return 0;
}

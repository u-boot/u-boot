/*
 * (C) Copyright 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <asm/processor.h>
#include <asm/cache.h>

int checkcpu(void)
{
#ifdef CONFIG_SH4A
	puts("CPU: SH-4A\n");
#else
	puts("CPU: SH4\n");
#endif
	return 0;
}

int cpu_init (void)
{
	return 0;
}

int cleanup_before_linux (void)
{
	disable_interrupts();
	return 0;
}

int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	disable_interrupts();
	reset_cpu (0);
	return 0;
}

void flush_cache (unsigned long addr, unsigned long size)
{
	dcache_invalid_range( addr , addr + size );
}

void icache_enable (void)
{
	cache_control(0);
}

void icache_disable (void)
{
	cache_control(1);
}

int icache_status (void)
{
	return 0;
}

void dcache_enable (void)
{
}

void dcache_disable (void)
{
}

int dcache_status (void)
{
	return 0;
}

int cpu_eth_init(bd_t *bis)
{
#ifdef CONFIG_SH_ETHER
	sh_eth_initialize(bis);
#endif
	return 0;
}

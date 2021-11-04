// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/global_data.h>

int cpu_has_64bit(void)
{
	return true;
}

void enable_caches(void)
{
	/* Not implemented */
}

void disable_caches(void)
{
	/* Not implemented */
}

int dcache_status(void)
{
	return true;
}

int x86_mp_init(void)
{
	/* Not implemented */
	return 0;
}

int x86_cpu_reinit_f(void)
{
	return 0;
}

int cpu_phys_address_size(void)
{
	return CONFIG_CPU_ADDR_BITS;
}

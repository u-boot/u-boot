// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <cpu_func.h>
#include <irq_func.h>

/*
 * cleanup_before_linux() is called just before we call Linux, it prepares the
 * processor for linux
 *
 * We disable interrupts and flush caches.
 */
int cleanup_before_linux(void)
{
	disable_interrupts();

	flush_dcache_all();
	invalidate_icache_all();

	return 0;
}

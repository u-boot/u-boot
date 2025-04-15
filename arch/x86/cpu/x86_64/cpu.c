// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <cpu_func.h>
#include <debug_uart.h>
#include <init.h>
#include <asm/cpu.h>
#include <asm/global_data.h>
#include <asm/processor-flags.h>

DECLARE_GLOBAL_DATA_PTR;

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

/* enable SSE features for hardware floating point */
static void setup_sse_features(void)
{
	asm ("mov %%cr4, %%rax\n" \
	"or  %0, %%rax\n" \
	"mov %%rax, %%cr4\n" \
	: : "i" (X86_CR4_OSFXSR | X86_CR4_OSXMMEXCPT) : "eax");
}

int x86_cpu_reinit_f(void)
{
	/* set the vendor to Intel so that native_calibrate_tsc() works */
	gd->arch.x86_vendor = X86_VENDOR_INTEL;
	gd->arch.has_mtrr = true;
	if (IS_ENABLED(CONFIG_X86_HARDFP))
		setup_sse_features();

	return 0;
}

int x86_cpu_init_f(void)
{
	return 0;
}

#ifdef CONFIG_DEBUG_UART_BOARD_INIT
void board_debug_uart_init(void)
{
	/* this was already done in SPL */
}
#endif

void x86_get_identity_for_timer(void)
{
	/* set the vendor to Intel so that native_calibrate_tsc() works */
	gd->arch.x86_vendor = X86_VENDOR_INTEL;
}

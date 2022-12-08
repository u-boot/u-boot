// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Sean Anderson <sean.anderson@seco.com>
 * Copyright 2014 Broadcom Corporation
 */

#include <common.h>

/*
 * Macro to force the compiler to *populate* memory (for an array or struct)
 * before passing the pointer to an inline assembly call.
 */
#define USE_PTR(ptr) *(const char (*)[]) (ptr)

#if defined(CONFIG_ARM64)
	#define SMH_TRAP "hlt #0xf000"
#elif defined(CONFIG_CPU_V7M)
	#define SMH_TRAP "bkpt #0xAB"
#elif defined(CONFIG_SYS_THUMB_BUILD)
	#define SMH_TRAP "svc #0xab"
#else
	#define SMH_TRAP "svc #0x123456"
#endif

/*
 * Call the handler
 */
long smh_trap(unsigned int sysnum, void *addr)
{
	register long result asm("r0");
	register void *_addr asm("r1") = addr;

	/*
	 * We need a memory clobber (aka compiler barrier) for two reasons:
	 * - The compiler needs to populate any data structures pointed to
	 *   by "addr" *before* the trap instruction is called.
	 * - At least the SYSREAD function puts the result into memory pointed
	 *   to by "addr", so the compiler must not use a cached version of
	 *   the previous content, after the call has finished.
	 */
	asm volatile (SMH_TRAP
		      : "=r" (result)
		      : "0"(sysnum), "r"(USE_PTR(_addr))
		      : "memory");

	return result;
}

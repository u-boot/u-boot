// SPDX-License-Identifier: GPL-2.0+
/*
 * Utility functions needed for (some) EABI conformant tool chains.
 *
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 */

#include <stdio.h>
#include <linux/stddef.h>
#include <linux/string.h>

int raise (int signum)
{
	/* Even if printf() is available, it's large. Punt it for SPL builds */
#if !defined(CONFIG_XPL_BUILD)
	printf("raise: Signal # %d caught\n", signum);
#endif
	return 0;
}

/* Dummy function to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
}

void __aeabi_unwind_cpp_pr1(void)
{
}

/* Copy memory like memcpy, but no return value required.  */
void __aeabi_memcpy(void *dest, const void *src, size_t n)
{
	(void) memcpy(dest, src, n);
}

void __aeabi_memcpy4(void *dest, const void *src, size_t n) __alias(__aeabi_memcpy);

void __aeabi_memcpy8(void *dest, const void *src, size_t n) __alias(__aeabi_memcpy);

void __aeabi_memset(void *dest, size_t n, int c)
{
	(void) memset(dest, c, n);
}

void __aeabi_memset4(void *dest, size_t n, int c) __alias(__aeabi_memset);

void __aeabi_memset8(void *dest, size_t n, int c) __alias(__aeabi_memset);

void __aeabi_memclr(void *dest, size_t n)
{
	(void) memset(dest, 0, n);
}

void __aeabi_memclr4(void *dest, size_t n) __alias(__aeabi_memclr);

void __aeabi_memclr8(void *dest, size_t n) __alias(__aeabi_memclr);

/*
 * Utility functions needed for (some) EABI conformant tool chains.
 *
 * (C) Copyright 2009 Wolfgang Denk <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

int raise (int signum)
{
	/* Even if printf() is available, it's large. Punt it for SPL builds */
#if !defined(CONFIG_SPL_BUILD)
	printf("raise: Signal # %d caught\n", signum);
#endif
	return 0;
}

/* Dummy function to avoid linker complaints */
void __aeabi_unwind_cpp_pr0(void)
{
};

void __aeabi_unwind_cpp_pr1(void)
{
};

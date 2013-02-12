/*
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 *
 * Copyright (C) 2003 Wolfgang Denk, DENX Software Engineering, wd@denx.de
 */

extern ulong uboot_end_data;
extern ulong uboot_end;

static inline unsigned long bss_start(void)
{
	extern ulong __bss_start;
	return (unsigned long) &__bss_start;
}

static inline unsigned long bss_end(void)
{
	extern ulong __bss_end;
	return (unsigned long) &__bss_end;
}

extern int incaip_set_cpuclk(void);

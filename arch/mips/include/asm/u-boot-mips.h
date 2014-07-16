/*
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (C) 2003 Wolfgang Denk, DENX Software Engineering, wd@denx.de
 */

static inline unsigned long bss_start(void)
{
	extern char __bss_start[];
	return (unsigned long) &__bss_start;
}

static inline unsigned long bss_end(void)
{
	extern ulong __bss_end;
	return (unsigned long) &__bss_end;
}

static inline unsigned long image_copy_end(void)
{
	extern char __image_copy_end[];
	return (unsigned long) &__image_copy_end;
}

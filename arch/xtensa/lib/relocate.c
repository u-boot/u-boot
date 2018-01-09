/*
 * Copyright (C) 2016 Cadence Design Systems Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <relocate.h>
#include <asm/sections.h>
#include <asm/string.h>

int clear_bss(void)
{
	size_t len = (size_t)&__bss_end - (size_t)&__bss_start;

	memset((void *)&__bss_start, 0x00, len);
	return 0;
}


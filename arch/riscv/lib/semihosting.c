// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2022 Ventana Micro Systems Inc.
 */

#include <common.h>

long smh_trap(int sysnum, void *addr)
{
	register int ret asm ("a0") = sysnum;
	register void *param0 asm ("a1") = addr;

	asm volatile (".align 4\n"
		".option push\n"
		".option norvc\n"

		"slli zero, zero, 0x1f\n"
		"ebreak\n"
		"srai zero, zero, 7\n"
		".option pop\n"
		: "+r" (ret) : "r" (param0) : "memory");

	return ret;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 */

#include <asm/u-boot.h>

unsigned long do_go_exec(ulong (*entry)(int, char * const []),
			 int argc, char *const argv[])
{
	cleanup_before_linux();

	return entry(argc, argv);
}

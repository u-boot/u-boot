// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, Bin Meng <bmeng.cn@gmail.com>
 *
 * From arch/x86/lib/asm-offsets.c
 *
 * This program is used to generate definitions needed by
 * assembly language modules.
 */

#include <common.h>
#include <linux/kbuild.h>

int main(void)
{
	DEFINE(GD_BOOT_HART, offsetof(gd_t, arch.boot_hart));

	return 0;
}

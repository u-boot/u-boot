/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Save cboot-related boot-time CPU state
 *
 * (C) Copyright 2015-2016 NVIDIA Corporation <www.nvidia.com>
 */

#include <config.h>
#include <linux/linkage.h>

.align 8
.globl	cboot_boot_x0
cboot_boot_x0:
	.dword 0

ENTRY(cboot_save_boot_params)
	adr	x8, cboot_boot_x0
	str	x0, [x8]
	b	save_boot_params_ret
ENDPROC(cboot_save_boot_params)

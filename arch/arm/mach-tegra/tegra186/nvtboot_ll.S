/*
 * Save nvtboot-related boot-time CPU state
 *
 * (C) Copyright 2015-2016 NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>

.align 8
.globl	nvtboot_boot_x0
nvtboot_boot_x0:
	.dword 0

ENTRY(save_boot_params)
	adr	x8, nvtboot_boot_x0
	str	x0, [x8]
	b	save_boot_params_ret
ENDPROC(save_boot_params)

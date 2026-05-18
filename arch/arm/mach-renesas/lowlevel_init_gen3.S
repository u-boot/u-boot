/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * arch/arm/cpu/armv8/rcar_gen3/lowlevel_init.S
 *	This file is lowlevel initialize routine.
 *
 * (C) Copyright 2015 Renesas Electronics Corporation
 *
 * This file is based on the arch/arm/cpu/armv8/start.S
 *
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

.align 8
.globl	rcar_atf_boot_args
rcar_atf_boot_args:
	.dword 0
	.dword 0
	.dword 0
	.dword 0

ENTRY(save_boot_params)
	adr	x8, rcar_atf_boot_args
	stp	x0, x1, [x8], #16
	stp	x2, x3, [x8], #16
	b	save_boot_params_ret
ENDPROC(save_boot_params)

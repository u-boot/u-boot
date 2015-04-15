/*
 * Copyright (C) 2015 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/linkage.h>
#include <mach/ssc-regs.h>

ENTRY(lowlevel_init)
	ldr	r1, = SSCC
	ldr	r0, [r1]
	bic	r0, r0, #SSCC_ON	@ L2 disable
	str	r0, [r1]
	mov	pc, lr
ENDPROC(lowlevel_init)

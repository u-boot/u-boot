/*
 * SoC-specific setup info
 *
 * (C) Copyright 2010,2011
 * NVIDIA Corporation <www.nvidia.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <version.h>
#include <linux/linkage.h>

	.align	5
ENTRY(reset_cpu)
	ldr	r1, rstctl			@ get addr for global reset
						@ reg
	ldr	r3, [r1]
	orr	r3, r3, #0x10
	str	r3, [r1]			@ force reset
	mov	r0, r0
_loop_forever:
	b	_loop_forever
rstctl:
	.word	PRM_RSTCTRL
ENDPROC(reset_cpu)

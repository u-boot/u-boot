/*
 * (C) Copyright 2011 ST-Ericsson
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mov	pc, lr
ENDPROC(lowlevel_init)

	.align	5
ENTRY(reset_cpu)
	ldr r0, =CFG_PRCMU_BASE
	ldr r1, =0x1
	str r1, [r0, #0x228]
_loop_forever:
	b	_loop_forever
ENDPROC(reset_cpu)

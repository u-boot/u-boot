/*
 * arch/arm/cpu/armv7/rmobile/lowlevel_init_ca15.S
 *     This file is lager low level initialize.
 *
 * Copyright (C) 2013 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier: GPL-2.0
 */

#include <config.h>
#include <linux/linkage.h>

ENTRY(lowlevel_init)
	mrc	p15, 0, r4, c0, c0, 5 /* mpidr */
	orr	r4, r4, r4, lsr #6
	and	r4, r4, #7 /* id 0-3 = ca15.0,1,2,3 */

	b do_lowlevel_init

	.pool

/*
 * CPU ID #1-#3 come here
 */
	.align  4
do_cpu_waiting:
	ldr	r1, =0xe6180000 /* sysc */
1:	ldr	r0, [r1, #0x20] /* sbar */
	tst	r0, r0
	beq	1b
	bx	r0

/*
 * Only CPU ID #0 comes here
 */
	.align  4
do_lowlevel_init:
	/* surpress wfe if ca15 */
	tst	r4, #4
	mrceq p15, 0, r0, c1, c0, 1	/* actlr */
	orreq r0, r0, #(1<<7)
	mcreq p15, 0, r0, c1, c0, 1
	/* and set l2 latency */
	mrceq p15, 1, r0, c9, c0, 2	/* l2ctlr */
	orreq r0, r0, #0x00000800
	orreq r0, r0, #0x00000003
	mcreq p15, 1, r0, c9, c0, 2

	ldr	r3, =(CONFIG_SYS_INIT_SP_ADDR)
	sub	sp, r3, #4
	str	lr, [sp]

	/* initialize system */
	bl s_init

	ldr	lr, [sp]
	mov	pc, lr
	nop
ENDPROC(lowlevel_init)
	.ltorg

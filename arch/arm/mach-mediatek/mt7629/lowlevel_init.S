/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018 MediaTek Inc.
 */

#include <linux/linkage.h>

ENTRY(lowlevel_init)

#ifndef CONFIG_SPL_BUILD
	/* Return to U-Boot via saved link register */
	mov	pc, lr
#else
	/*
	 * Arch timer :
	 * set CNTFRQ = 20Mhz, set CNTVOFF = 0
	 */
	movw	r0, #0x2d00
	movt	r0, #0x131
	mcr	p15, 0, r0, c14, c0, 0

	/* enable SMP bit */
	mrc	p15, 0, r0, c1, c0, 1
	orr	r0, r0, #0x40
	mcr	p15, 0, r0, c1, c0, 1

	/* if MP core, handle secondary cores */
	mrc	p15, 0, r0, c0, c0, 5
	ands	r1, r0, #0x40000000
	bne	go			@ Go if UP
	ands	r0, r0, #0x0f
	beq	go			@ Go if core0 on primary core tile
	b	secondary

go:
	/* master CPU */
	mov	pc, lr

secondary:
	/* read slave CPU number into r0 firstly */
	mrc	p15, 0, r0, c0, c0, 5
	and	r0, r0, #0x0f

loop:
	dsb
	isb
	wfi				@Zzz...
	b	loop
#endif
ENDPROC(lowlevel_init)

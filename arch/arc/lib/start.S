/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/arcregs.h>

ENTRY(_start)
	/* Setup interrupt vector base that matches "__text_start" */
	sr	__ivt_start, [ARC_AUX_INTR_VEC_BASE]

	/* Setup stack- and frame-pointers */
	mov	%sp, CONFIG_SYS_INIT_SP_ADDR
	mov	%fp, %sp

	/* Unconditionally disable caches */
#ifdef CONFIG_ISA_ARCV2
	bl	slc_flush
	bl	slc_disable
#endif
	bl	flush_dcache_all
	bl	dcache_disable
	bl	icache_disable

	/* Allocate and zero GD, update SP */
	mov	%r0, %sp
	bl	board_init_f_mem

	/* Update stack- and frame-pointers */
	mov	%sp, %r0
	mov	%fp, %sp

	/* Zero the one and only argument of "board_init_f" */
	mov_s	%r0, 0
	j	board_init_f
ENDPROC(_start)

/*
 * void board_init_f_r_trampoline(stack-pointer address)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r0 = new stack-pointer
 */
ENTRY(board_init_f_r_trampoline)
	/* Set up the stack- and frame-pointers */
	mov	%sp, %r0
	mov	%fp, %sp

	/* Update position of intterupt vector table */
	lr	%r0, [ARC_AUX_INTR_VEC_BASE]
	ld	%r1, [%r25, GD_RELOC_OFF]
	add	%r0, %r0, %r1
	sr	%r0, [ARC_AUX_INTR_VEC_BASE]

	/* Re-enter U-Boot by calling board_init_f_r */
	j	board_init_f_r
ENDPROC(board_init_f_r_trampoline)

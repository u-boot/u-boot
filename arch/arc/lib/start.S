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

	/* Setup stack pointer */
	mov	%sp, CONFIG_SYS_INIT_SP_ADDR
	mov	%fp, %sp

	/* Clear bss */
	mov	%r0, __bss_start
	mov	%r1, __bss_end

clear_bss:
	st.ab	0, [%r0, 4]
	brlt	%r0, %r1, clear_bss

	/* Zero the one and only argument of "board_init_f" */
	mov_s	%r0, 0
	j	board_init_f
ENDPROC(_start)

/*
 * void relocate_code (addr_sp, gd, addr_moni)
 *
 * This "function" does not return, instead it continues in RAM
 * after relocating the monitor code.
 *
 * r0 = start_addr_sp
 * r1 = new__gd
 * r2 = relocaddr
 */
ENTRY(relocate_code)
	/*
	 * r0-r12 might be clobbered by C functions
	 * so we use r13-r16 for storage here
	 */
	mov	%r13, %r0		/* save addr_sp */
	mov	%r14, %r1		/* save addr of gd */
	mov	%r15, %r2		/* save addr of destination */

	mov	%r16, %r2		/* %r9 - relocation offset */
	sub	%r16, %r16, __image_copy_start

/* Set up the stack */
stack_setup:
	mov	%sp, %r13
	mov	%fp, %sp

/* Check if monitor is loaded right in place for relocation */
	mov	%r0, __image_copy_start
	cmp	%r0, %r15		/* skip relocation if code loaded */
	bz	do_board_init_r		/* in target location already */

/* Copy data (__image_copy_start - __image_copy_end) to new location */
	mov	%r1, %r15
	mov	%r2, __image_copy_end
	sub	%r2, %r2, %r0		/* r3 <- amount of bytes to copy */
	asr	%r2, %r2, 2		/* r3 <- amount of words to copy */
	mov	%lp_count, %r2
	lp	copy_end
	ld.ab	%r2,[%r0,4]
	st.ab	%r2,[%r1,4]
copy_end:

/* Fix relocations related issues */
	bl	do_elf_reloc_fixups
#ifndef CONFIG_SYS_ICACHE_OFF
	bl	invalidate_icache_all
#endif
#ifndef CONFIG_SYS_DCACHE_OFF
	bl	flush_dcache_all
#endif

/* Update position of intterupt vector table */
	lr	%r0, [ARC_AUX_INTR_VEC_BASE]	/* Read current position */
	add	%r0, %r0, %r16			/* Update address */
	sr	%r0, [ARC_AUX_INTR_VEC_BASE]	/* Write new position */

do_board_init_r:
/* Prepare for exection of "board_init_r" in relocated monitor */
	mov	%r2, board_init_r	/* old address of "board_init_r()" */
	add	%r2, %r2, %r16		/* new address of "board_init_r()" */
	mov	%r0, %r14		/* 1-st parameter: gd_t */
	mov	%r1, %r15		/* 2-nd parameter: dest_addr */
	j	[%r2]
ENDPROC(relocate_code)

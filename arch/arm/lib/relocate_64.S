/*
 * relocate - common relocation function for AArch64 U-Boot
 *
 * (C) Copyright 2013
 * Albert ARIBAUD <albert.u.boot@aribaud.net>
 * David Feng <fenghua@phytium.com.cn>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm-offsets.h>
#include <config.h>
#include <linux/linkage.h>
#include <asm/macro.h>

/*
 * void relocate_code (addr_moni)
 *
 * This function relocates the monitor code.
 * x0 holds the destination address.
 */
ENTRY(relocate_code)
	stp	x29, x30, [sp, #-32]!	/* create a stack frame */
	mov	x29, sp
	str	x0, [sp, #16]
	/*
	 * Copy u-boot from flash to RAM
	 */
	ldr	x1, =__image_copy_start	/* x1 <- SRC &__image_copy_start */
	subs	x9, x0, x1		/* x9 <- relocation offset */
	b.eq	relocate_done		/* skip relocation */
	ldr	x2, =__image_copy_end	/* x2 <- SRC &__image_copy_end */

copy_loop:
	ldp	x10, x11, [x1], #16	/* copy from source address [x1] */
	stp	x10, x11, [x0], #16	/* copy to   target address [x0] */
	cmp	x1, x2			/* until source end address [x2] */
	b.lo	copy_loop
	str	x0, [sp, #24]

	/*
	 * Fix .rela.dyn relocations
	 */
	ldr	x2, =__rel_dyn_start	/* x2 <- SRC &__rel_dyn_start */
	ldr	x3, =__rel_dyn_end	/* x3 <- SRC &__rel_dyn_end */
fixloop:
	ldp	x0, x1, [x2], #16	/* (x0,x1) <- (SRC location, fixup) */
	ldr	x4, [x2], #8		/* x4 <- addend */
	and	x1, x1, #0xffffffff
	cmp	x1, #1027		/* relative fixup? */
	bne	fixnext

	/* relative fix: store addend plus offset at dest location */
	add	x0, x0, x9
	add	x4, x4, x9
	str	x4, [x0]
fixnext:
	cmp	x2, x3
	b.lo	fixloop

relocate_done:
	switch_el x1, 3f, 2f, 1f
	bl	hang
3:	mrs	x0, sctlr_el3
	b	0f
2:	mrs	x0, sctlr_el2
	b	0f
1:	mrs	x0, sctlr_el1
0:	tbz	w0, #2, 5f	/* skip flushing cache if disabled */
	tbz	w0, #12, 4f	/* invalide i-cache is enabled */
	ic	iallu		/* i-cache invalidate all */
	isb	sy
4:	ldp	x0, x1, [sp, #16]
	bl	__asm_flush_dcache_range
5:	ldp	x29, x30, [sp],#16
	ret
ENDPROC(relocate_code)

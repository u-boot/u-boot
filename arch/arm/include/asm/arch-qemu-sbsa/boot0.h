/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * sbsa-ref starts U-Boot in XIP memory. Need to relocate U-Boot
 * to DRAM which is already up. Instead of using SPL this simple loader
 * is being used.
 */
relocate_check:
	/* x0 contains the pointer to FDT provided by ATF */
	adr	x1, _start		/* x1 <- Runtime value of _start */
	ldr	x2, _TEXT_BASE		/* x2 <- Linked value of _start */
	subs	x9, x1, x2		/* x9 <- Run-vs-link offset */
	beq	reset

	adrp	x1, __image_copy_start		/* x2 <- address bits [31:12] */
	add	x1, x1, :lo12:__image_copy_start/* x2 <- address bits [11:00] */
	adrp	x3, __image_copy_end		/* x3 <- address bits [31:12] */
	add	x3, x3, :lo12:__image_copy_end	/* x3 <- address bits [11:00] */
	add	x3, x3, #0x100000		/* 1 MiB for the DTB found at _end */

copy_loop:
	ldp	x10, x11, [x1], #16	/* copy from source address [x1] */
	stp	x10, x11, [x2], #16	/* copy to   target address [x2] */
	cmp	x1, x3			/* until source end address [x3] */
	b.lo	copy_loop

	isb
	ldr	x2, _TEXT_BASE		/* x2 <- Linked value of _start */
	br	x2			/* Jump to linked address */
	/* Never reaches this point */
1:
	wfi
	b 1b

relocate_done:
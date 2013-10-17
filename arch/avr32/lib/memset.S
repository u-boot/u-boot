/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

	/*
	 * r12:	void *b
	 * r11:	int c
	 * r10:	size_t len
	 *
	 * Returns b in r12
	 */
	.section .text.memset, "ax", @progbits

	.global	memset
	.type	memset, @function
	.align	2
memset:
	mov	r9, r12
	mov	r8, r12
	or	r11, r11, r11 << 8
	andl	r9, 3, COH
	brne	1f

2:	or	r11, r11, r11 << 16
	sub	r10, 4
	brlt	5f

	/* Let's do some real work */
4:	st.w	r8++, r11
	sub	r10, 4
	brge	4b

	/*
	 * When we get here, we've got less than 4 bytes to set. r10
	 * might be negative.
	 */
5:	sub	r10, -4
	reteq	r12

	/* Fastpath ends here, exactly 32 bytes from memset */

	/* Handle unaligned count or pointer */
	bld	r10, 1
	brcc	6f
	st.b	r8++, r11
	st.b	r8++, r11
	bld	r10, 0
	retcc	r12
6:	st.b	r8++, r11
	mov	pc, lr

	/* Handle unaligned pointer */
1:	sub	r10, 4
	brlt	5b
	add	r10, r9
	lsl	r9, 1
	add	pc, r9
	st.b	r8++, r11
	st.b	r8++, r11
	st.b	r8++, r11
	rjmp	2b

	.size	memset, . - memset

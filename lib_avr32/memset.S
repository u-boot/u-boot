/*
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

	/*
	 * r12:	void *b
	 * r11:	int c
	 * r10:	size_t len
	 *
	 * Returns b in r12
	 */
	.text

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

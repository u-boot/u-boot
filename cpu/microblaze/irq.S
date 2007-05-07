/*
 * (C) Copyright 2007 Michal Simek
 *
 * Michal  SIMEK <monstr@monstr.eu>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
	.text
	.global _interrupt_handler
_interrupt_handler:
	addi	r1, r1, -4
	swi	r2, r1, 0
	addi	r1, r1, -4
	swi	r3, r1, 0
	addi	r1, r1, -4
	swi	r4, r1, 0
	addi	r1, r1, -4
	swi	r5, r1, 0
	addi	r1, r1, -4
	swi	r6, r1, 0
	addi	r1, r1, -4
	swi	r7, r1, 0
	addi	r1, r1, -4
	swi	r8, r1, 0
	addi	r1, r1, -4
	swi	r9, r1, 0
	addi	r1, r1, -4
	swi	r10, r1, 0
	addi	r1, r1, -4
	swi	r11, r1, 0
	addi	r1, r1, -4
	swi	r12, r1, 0
	addi	r1, r1, -4
	swi	r13, r1, 0
	addi	r1, r1, -4
	swi	r14, r1, 0
	addi	r1, r1, -4
	swi	r15, r1, 0
	addi	r1, r1, -4
	swi	r16, r1, 0
	addi	r1, r1, -4
	swi	r17, r1, 0
	addi	r1, r1, -4
	swi	r18, r1, 0
	addi	r1, r1, -4
	swi	r19, r1, 0
	addi	r1, r1, -4
	swi	r20, r1, 0
	addi	r1, r1, -4
	swi	r21, r1, 0
	addi	r1, r1, -4
	swi	r22, r1, 0
	addi	r1, r1, -4
	swi	r23, r1, 0
	addi	r1, r1, -4
	swi	r24, r1, 0
	addi	r1, r1, -4
	swi	r25, r1, 0
	addi	r1, r1, -4
	swi	r26, r1, 0
	addi	r1, r1, -4
	swi	r27, r1, 0
	addi	r1, r1, -4
	swi	r28, r1, 0
	addi	r1, r1, -4
	swi	r29, r1, 0
	addi	r1, r1, -4
	swi	r30, r1, 0
	addi	r1, r1, -4
	swi	r31, r1, 0
	brlid	r15, interrupt_handler
	nop
	nop
	lwi	r31, r1, 0
	addi	r1, r1, 4
	lwi	r30, r1, 0
	addi	r1, r1, 4
	lwi	r29, r1, 0
	addi	r1, r1, 4
	lwi	r28, r1, 0
	addi	r1, r1, 4
	lwi	r27, r1, 0
	addi	r1, r1, 4
	lwi	r26, r1, 0
	addi	r1, r1, 4
	lwi	r25, r1, 0
	addi	r1, r1, 4
	lwi	r24, r1, 0
	addi	r1, r1, 4
	lwi	r23, r1, 0
	addi	r1, r1, 4
	lwi	r22, r1, 0
	addi	r1, r1, 4
	lwi	r21, r1, 0
	addi	r1, r1, 4
	lwi	r20, r1, 0
	addi	r1, r1, 4
	lwi	r19, r1, 0
	addi	r1, r1, 4
	lwi	r18, r1, 0
	addi	r1, r1, 4
	lwi	r17, r1, 0
	addi	r1, r1, 4
	lwi	r16, r1, 0
	addi	r1, r1, 4
	lwi	r15, r1, 0
	addi	r1, r1, 4
	lwi	r14, r1, 0
	addi	r1, r1, 4
	lwi	r13, r1, 0
	addi	r1, r1, 4
	lwi	r12, r1, 0
	addi	r1, r1, 4
	lwi	r11, r1, 0
	addi	r1, r1, 4
	lwi	r10, r1, 0
	addi	r1, r1, 4
	lwi	r9, r1, 0
	addi	r1, r1, 4
	lwi	r8, r1, 0
	addi	r1, r1, 4
	lwi	r7, r1, 0
	addi	r1, r1, 4
	lwi	r6, r1, 0
	addi	r1, r1, 4
	lwi	r5, r1, 0
	addi	r1, r1, 4
	lwi	r4, r1, 0
	addi	r1, r1, 4
	lwi	r3, r1, 0
	addi	r1, r1, 4
	lwi	r2, r1, 0
	addi	r1, r1, 4

	/* enable_interrupt */
	msrset	r0, 2
	bra	r14
	nop
	nop
	.size _interrupt_handler,.-_interrupt_handler

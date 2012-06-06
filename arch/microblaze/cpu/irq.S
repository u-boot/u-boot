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
#include <asm/asm.h>
	.text
	.global _interrupt_handler
_interrupt_handler:
	swi	r2, r1, -4
	swi	r3, r1, -8
	swi	r4, r1, -12
	swi	r5, r1, -16
	swi	r6, r1, -20
	swi	r7, r1, -24
	swi	r8, r1, -28
	swi	r9, r1, -32
	swi	r10, r1, -36
	swi	r11, r1, -40
	swi	r12, r1, -44
	swi	r13, r1, -48
	swi	r14, r1, -52
	swi	r15, r1, -56
	swi	r16, r1, -60
	swi	r17, r1, -64
	swi	r18, r1, -68
	swi	r19, r1, -72
	swi	r20, r1, -76
	swi	r21, r1, -80
	swi	r22, r1, -84
	swi	r23, r1, -88
	swi	r24, r1, -92
	swi	r25, r1, -96
	swi	r26, r1, -100
	swi	r27, r1, -104
	swi	r28, r1, -108
	swi	r29, r1, -112
	swi	r30, r1, -116
	swi	r31, r1, -120
	addik	r1, r1, -124
	brlid	r15, interrupt_handler
	nop
	addik	r1, r1, 124
	lwi	r31, r1, -120
	lwi	r30, r1, -116
	lwi	r29, r1, -112
	lwi	r28, r1, -108
	lwi	r27, r1, -104
	lwi	r26, r1, -100
	lwi	r25, r1, -96
	lwi	r24, r1, -92
	lwi	r23, r1, -88
	lwi	r22, r1, -84
	lwi	r21, r1, -80
	lwi	r20, r1, -76
	lwi	r19, r1, -72
	lwi	r18, r1, -68
	lwi	r17, r1, -64
	lwi	r16, r1, -60
	lwi	r15, r1, -56
	lwi	r14, r1, -52
	lwi	r13, r1, -48
	lwi	r12, r1, -44
	lwi	r11, r1, -40
	lwi	r10, r1, -36
	lwi	r9, r1, -32
	lwi	r8, r1, -28
	lwi	r7, r1, -24
	lwi	r6, r1, -20
	lwi	r5, r1, -16
	lwi	r4, r1, -12
	lwi	r3, r1, -8
	lwi	r2, r1, -4

	rtid	r14, 0
	nop
	.size _interrupt_handler,.-_interrupt_handler

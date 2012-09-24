/*
 * (C) Copyright 2011, Stefan Kristiansson <stefan.kristiansson@saunalahti.fi>
 * (C) Copyright 2011, Julius Baxter <julius@opencores.org>
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

#include <config.h>
#include <asm-offsets.h>
#include <asm/spr-defs.h>

#define EXCEPTION_STACK_SIZE (128+128)

#define HANDLE_EXCEPTION			\
	l.addi	r1, r1, -EXCEPTION_STACK_SIZE	;\
	l.sw	0x00(r1), r2			;\
	l.sw	0x1c(r1), r9			;\
	l.movhi r2,hi(_exception_handler)       ;\
	l.ori   r2,r2,lo(_exception_handler)    ;\
	l.jalr	r2              		;\
	 l.nop					;\
	l.lwz 	r9, 0x1c(r1)			;\
	l.addi	r1, r1, EXCEPTION_STACK_SIZE	;\
	l.rfe					;\
	 l.nop

	.section .vectors, "ax"
	.global __reset

	/* reset */
	.org	0x100
__reset:
	/* there is no guarantee r0 is hardwired to zero, clear it here */
	l.andi	r0, r0, 0
	/* reset stack and frame pointers */
	l.andi	r1, r0, 0
	l.andi	r2, r0, 0

	/* set supervisor mode */
	l.ori	r3,r0,SPR_SR_SM
	l.mtspr	r0,r3,SPR_SR

	/* Relocate u-boot */
	l.movhi	r3,hi(__start)		/* source start address */
	l.ori	r3,r3,lo(__start)
	l.movhi	r4,hi(_stext)		/* dest start address */
	l.ori	r4,r4,lo(_stext)
	l.movhi	r5,hi(__end)		/* dest end address */
	l.ori	r5,r5,lo(__end)

.L_reloc:
	l.lwz	r6,0(r3)
	l.sw	0(r4),r6
	l.addi	r3,r3,4
	l.sfltu	r4,r5
	l.bf	.L_reloc
	 l.addi	r4,r4,4			/* delay slot */

#ifdef CONFIG_SYS_RELOCATE_VECTORS
	/* Relocate vectors from 0xf0000000 to 0x00000000 */
	l.movhi r4, 0xf000 /* source */
	l.movhi r5, 0      /* destination */
	l.addi	r6, r5, CONFIG_SYS_VECTORS_LEN /* length */
.L_relocvectors:
	l.lwz	r7, 0(r4)
	l.sw	0(r5), r7
	l.addi	r5, r5, 4
	l.sfeq	r5,r6
	l.bnf	.L_relocvectors
	 l.addi	r4,r4, 4
#endif
	l.movhi	r4,hi(_start)
	l.ori	r4,r4,lo(_start)
	l.jr	r4
	 l.nop

	/* bus error */
	.org	0x200
	HANDLE_EXCEPTION

	/* data page fault */
	.org	0x300
	HANDLE_EXCEPTION

	/* instruction page fault */
	.org	0x400
	HANDLE_EXCEPTION

	/* tick timer */
	.org	0x500
	HANDLE_EXCEPTION

	/* alignment */
	.org	0x600
	HANDLE_EXCEPTION

	/* illegal instruction */
	.org	0x700
	HANDLE_EXCEPTION

	/* external interrupt */
	.org	0x800
	HANDLE_EXCEPTION

	/* D-TLB miss */
	.org	0x900
	HANDLE_EXCEPTION

	/* I-TLB miss */
	.org	0xa00
	HANDLE_EXCEPTION

	/* range */
	.org	0xb00
	HANDLE_EXCEPTION

	/* system call */
	.org	0xc00
	HANDLE_EXCEPTION

	/* floating point */
	.org	0xd00
	HANDLE_EXCEPTION

	/* trap */
	.org	0xe00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0xf00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1100
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1200
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1300
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1400
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1500
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1600
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1700
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1800
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1900
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1a00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1b00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1c00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1d00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1e00
	HANDLE_EXCEPTION

	/* reserved */
	.org	0x1f00
	HANDLE_EXCEPTION

	/* Startup routine */
	.text
	.global _start
_start:
	/* Init stack and frame pointers */
	l.movhi	r1, hi(CONFIG_SYS_INIT_SP_ADDR)
	l.ori	r1, r1, lo(CONFIG_SYS_INIT_SP_ADDR)
	l.or	r2, r0, r1

	/* clear BSS segments */
	l.movhi	r4, hi(_bss_start)
	l.ori	r4, r4, lo(_bss_start)
	l.movhi	r5, hi(_bss_end)
	l.ori	r5, r5, lo(_bss_end)
.L_clear_bss:
	l.sw	0(r4), r0
	l.sfltu	r4,r5
	l.bf	.L_clear_bss
	 l.addi	r4,r4,4

	/* Reset registers before jumping to board_init */
	l.andi	r3, r0, 0
	l.andi	r4, r0, 0
	l.andi	r5, r0, 0
	l.andi	r6, r0, 0
	l.andi	r7, r0, 0
	l.andi	r8, r0, 0
	l.andi	r9, r0, 0
	l.andi	r10, r0, 0
	l.andi	r11, r0, 0
	l.andi	r12, r0, 0
	l.andi	r13, r0, 0
	l.andi	r14, r0, 0
	l.andi	r15, r0, 0
	l.andi	r17, r0, 0
	l.andi	r18, r0, 0
	l.andi	r19, r0, 0
	l.andi	r20, r0, 0
	l.andi	r21, r0, 0
	l.andi	r22, r0, 0
	l.andi	r23, r0, 0
	l.andi	r24, r0, 0
	l.andi	r25, r0, 0
	l.andi	r26, r0, 0
	l.andi	r27, r0, 0
	l.andi	r28, r0, 0
	l.andi	r29, r0, 0
	l.andi	r30, r0, 0
	l.andi	r31, r0, 0

	l.j	board_init
	 l.nop

	.size	_start, .-_start

/*
 * Store state onto stack and call the real exception handler
 */
	.section .text
	.extern	exception_handler
	.type	_exception_handler,@function

_exception_handler:
	/* Store state (r2 and r9 already saved)*/
	l.sw	0x04(r1), r3
	l.sw	0x08(r1), r4
	l.sw	0x0c(r1), r5
	l.sw	0x10(r1), r6
	l.sw	0x14(r1), r7
	l.sw	0x18(r1), r8
	l.sw	0x20(r1), r10
	l.sw	0x24(r1), r11
	l.sw	0x28(r1), r12
	l.sw	0x2c(r1), r13
	l.sw	0x30(r1), r14
	l.sw	0x34(r1), r15
	l.sw	0x38(r1), r16
	l.sw	0x3c(r1), r17
	l.sw	0x40(r1), r18
	l.sw	0x44(r1), r19
	l.sw	0x48(r1), r20
	l.sw	0x4c(r1), r21
	l.sw	0x50(r1), r22
	l.sw	0x54(r1), r23
	l.sw	0x58(r1), r24
	l.sw	0x5c(r1), r25
	l.sw	0x60(r1), r26
	l.sw	0x64(r1), r27
	l.sw	0x68(r1), r28
	l.sw	0x6c(r1), r29
	l.sw	0x70(r1), r30
	l.sw	0x74(r1), r31

	/* Save return address */
	l.or	r14, r0, r9
	/* Call exception handler with the link address as argument */
	l.jal	exception_handler
	 l.or	r3, r0, r14
	/* Load return address */
	l.or	r9, r0, r14

	/* Restore state */
	l.lwz	r2, 0x00(r1)
	l.lwz	r3, 0x04(r1)
	l.lwz	r4, 0x08(r1)
	l.lwz	r5, 0x0c(r1)
	l.lwz	r6, 0x10(r1)
	l.lwz	r7, 0x14(r1)
	l.lwz	r8, 0x18(r1)
	l.lwz	r10, 0x20(r1)
	l.lwz	r11, 0x24(r1)
	l.lwz	r12, 0x28(r1)
	l.lwz	r13, 0x2c(r1)
	l.lwz	r14, 0x30(r1)
	l.lwz	r15, 0x34(r1)
	l.lwz	r16, 0x38(r1)
	l.lwz	r17, 0x3c(r1)
	l.lwz	r18, 0x40(r1)
	l.lwz	r19, 0x44(r1)
	l.lwz	r20, 0x48(r1)
	l.lwz	r21, 0x4c(r1)
	l.lwz	r22, 0x50(r1)
	l.lwz	r23, 0x54(r1)
	l.lwz	r24, 0x58(r1)
	l.lwz	r25, 0x5c(r1)
	l.lwz	r26, 0x60(r1)
	l.lwz	r27, 0x64(r1)
	l.lwz	r28, 0x68(r1)
	l.lwz	r29, 0x6c(r1)
	l.lwz	r30, 0x70(r1)
	l.lwz	r31, 0x74(r1)
	l.jr	r9
	 l.nop

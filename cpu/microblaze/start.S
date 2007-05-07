/*
 * (C) Copyright 2007 Michal Simek
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Michal  SIMEK <monstr@monstr.eu>
 * Yasushi SHOJI <yashi@atmark-techno.com>
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
	.global _start
_start:
	mts	rmsr, r0	/* disable cache */
	addi	r1, r0, CFG_INIT_SP_OFFSET
	addi	r1, r1, -4	/* Decrement SP to top of memory */
	/* add opcode instruction for 32bit jump - 2 instruction imm & brai*/
	addi	r6, r0, 0xb000	/* hex b000 opcode imm */
	bslli	r6, r6, 16	/* shift */
	swi	r6, r0, 0x0	/* reset address */
	swi	r6, r0, 0x8	/* user vector exception */
	swi	r6, r0, 0x10	/* interrupt */
	swi	r6, r0, 0x20	/* hardware exception */

	addi	r6, r0, 0xb808	/* hew b808 opcode brai*/
	bslli	r6, r6, 16
	swi	r6, r0, 0x4	/* reset address */
	swi	r6, r0, 0xC	/* user vector exception */
	swi	r6, r0, 0x14	/* interrupt */
	swi	r6, r0, 0x24	/* hardware exception */

#ifdef CFG_RESET_ADDRESS
	/* reset address */
	addik	r6, r0, CFG_RESET_ADDRESS
	sw	r6, r1, r0
	lhu	r7, r1, r0
	shi	r7, r0, 0x2
	shi	r6, r0, 0x6
/*
 * Copy U-Boot code to TEXT_BASE
 * solve problem with sbrk_base
 */
#if (CFG_RESET_ADDRESS != TEXT_BASE)
	addi	r4, r0, __end
	addi	r5, r0, __text_start
	rsub	r4, r5, r4	/* size = __end - __text_start */
	addi	r6, r0, CFG_RESET_ADDRESS	/* source address */
	addi	r7, r0, 0	/* counter */
4:
	lw	r8, r6, r7
	sw	r8, r5, r7
	addi	r7, r7, 0x4
	cmp	r8, r4, r7
	blti	r8, 4b
#endif
#endif

#ifdef CFG_USR_EXCEP
	/* user_vector_exception */
	addik	r6, r0, _exception_handler
	sw	r6, r1, r0
	lhu	r7, r1, r0
	shi	r7, r0, 0xa
	shi	r6, r0, 0xe
#endif

#ifdef CFG_INTC_0
	/* interrupt_handler */
	addik	r6, r0, _interrupt_handler
	sw	r6, r1, r0
	lhu	r7, r1, r0
	shi	r7, r0, 0x12
	shi	r6, r0, 0x16
#endif

	/* hardware exception */
	addik	r6, r0, _hw_exception_handler
	sw	r6, r1, r0
	lhu	r7, r1, r0
	shi	r7, r0, 0x22
	shi	r6, r0, 0x26

	/* enable instruction and data cache */
	mfs	r12, rmsr
	ori	r12, r12, 0xa0
	mts	rmsr, r12

clear_bss:
	/* clear BSS segments */
	addi	r5, r0, __bss_start
	addi	r4, r0, __bss_end
	cmp	r6, r5, r4
	beqi	r6, 3f
2:
	swi     r0, r5, 0 /* write zero to loc */
	addi    r5, r5, 4 /* increment to next loc */
	cmp     r6, r5, r4 /* check if we have reach the end */
	bnei    r6, 2b
3:	/* jumping to board_init */
	brai	board_init
1:	bri	1b

/*
 * Read 16bit little endian
 */
	.text
	.global	in16
	.ent	in16
	.align	2
in16:	lhu	r3, r0, r5
	bslli	r4, r3, 8
	bsrli	r3, r3, 8
	andi	r4, r4, 0xffff
	or	r3, r3, r4
	rtsd	r15, 8
	sext16	r3, r3
	.end	in16

/*
 * Write 16bit little endian
 * first parameter(r5) - address, second(r6) - short value
 */
	.text
	.global	out16
	.ent	out16
	.align	2
out16:	bslli	r3, r6, 8
	bsrli	r6, r6, 8
	andi	r3, r3, 0xffff
	or	r3, r3, r6
	sh	r3, r0, r5
	rtsd	r15, 8
	or	r0, r0, r0
	.end	out16

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

#include <asm-offsets.h>
#include <config.h>

	.text
	.global _start
_start:
	/*
	 * reserve registers:
	 * r10: Stores little/big endian offset for vectors
	 * r2: Stores imm opcode
	 * r3: Stores brai opcode
	 */

	mts	rmsr, r0	/* disable cache */
	addi	r1, r0, CONFIG_SYS_INIT_SP_OFFSET
	addi	r1, r1, -4	/* Decrement SP to top of memory */

	/* Find-out if u-boot is running on BIG/LITTLE endian platform
	 * There are some steps which is necessary to keep in mind:
	 * 1. Setup offset value to r6
	 * 2. Store word offset value to address 0x0
	 * 3. Load just byte from address 0x0
	 * 4a) LITTLE endian - r10 contains 0x2 because it is the smallest
	 *     value that's why is on address 0x0
	 * 4b) BIG endian - r10 contains 0x0 because 0x2 offset is on addr 0x3
	 */
	addik	r6, r0, 0x2 /* BIG/LITTLE endian offset */
	lwi	r7, r0, 0x28
	swi	r6, r0, 0x28 /* used first unused MB vector */
	lbui	r10, r0, 0x28 /* used first unused MB vector */
	swi	r7, r0, 0x28

	/* add opcode instruction for 32bit jump - 2 instruction imm & brai */
	addi	r2, r0, 0xb0000000	/* hex b000 opcode imm */
	addi	r3, r0, 0xb8080000	/* hew b808 opcode brai */

#ifdef CONFIG_SYS_RESET_ADDRESS
	/* reset address */
	swi	r2, r0, 0x0	/* reset address - imm opcode */
	swi	r3, r0, 0x4	/* reset address - brai opcode */

	addik	r6, r0, CONFIG_SYS_RESET_ADDRESS
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x2
	sh	r7, r0, r8
	rsubi	r8, r10, 0x6
	sh	r6, r0, r8
#endif

#ifdef CONFIG_SYS_USR_EXCEP
	/* user_vector_exception */
	swi	r2, r0, 0x8	/* user vector exception - imm opcode */
	swi	r3, r0, 0xC	/* user vector exception - brai opcode */

	addik	r6, r0, _exception_handler
	sw	r6, r1, r0
	/*
	 * BIG ENDIAN memory map for user exception
	 * 0x8: 0xB000XXXX
	 * 0xC: 0xB808XXXX
	 *
	 * then it is necessary to count address for storing the most significant
	 * 16bits from _exception_handler address and copy it to
	 * 0xa address. Big endian use offset in r10=0 that's why is it just
	 * 0xa address. The same is done for the least significant 16 bits
	 * for 0xe address.
	 *
	 * LITTLE ENDIAN memory map for user exception
	 * 0x8: 0xXXXX00B0
	 * 0xC: 0xXXXX08B8
	 *
	 * Offset is for little endian setup to 0x2. rsubi instruction decrease
	 * address value to ensure that points to proper place which is
	 * 0x8 for the most significant 16 bits and
	 * 0xC for the least significant 16 bits
	 */
	lhu	r7, r1, r10
	rsubi	r8, r10, 0xa
	sh	r7, r0, r8
	rsubi	r8, r10, 0xe
	sh	r6, r0, r8
#endif

	/* interrupt_handler */
	swi	r2, r0, 0x10	/* interrupt - imm opcode */
	swi	r3, r0, 0x14	/* interrupt - brai opcode */

	addik	r6, r0, _interrupt_handler
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x12
	sh	r7, r0, r8
	rsubi	r8, r10, 0x16
	sh	r6, r0, r8

	/* hardware exception */
	swi	r2, r0, 0x20	/* hardware exception - imm opcode */
	swi	r3, r0, 0x24	/* hardware exception - brai opcode */

	addik	r6, r0, _hw_exception_handler
	sw	r6, r1, r0
	lhu	r7, r1, r10
	rsubi	r8, r10, 0x22
	sh	r7, r0, r8
	rsubi	r8, r10, 0x26
	sh	r6, r0, r8

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
	brai	board_init_f
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

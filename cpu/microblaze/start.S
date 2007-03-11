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

	/* jumping to board_init */
	brai	board_init
1:	bri	1b

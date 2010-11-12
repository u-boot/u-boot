/*
 * (C) Copyright 2001
 * Thomas Koeller, tkoeller@gmx.net
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

#ifndef	__ASSEMBLY__
#define __ASSEMBLY__	1
#endif

#include <asm-offsets.h>
#include <config.h>
#include <asm/processor.h>
#include <mpc824x.h>
#include <ppc_asm.tmpl>

#if defined(USE_DINK32)
  /* We are running from RAM, so do not clear the MCCR1_MEMGO bit! */
  #define MCCR1VAL ((CONFIG_SYS_ROMNAL << MCCR1_ROMNAL_SHIFT) | (CONFIG_SYS_ROMFAL << MCCR1_ROMFAL_SHIFT) | MCCR1_MEMGO)
#else
  #define MCCR1VAL (CONFIG_SYS_ROMNAL << MCCR1_ROMNAL_SHIFT) | (CONFIG_SYS_ROMFAL << MCCR1_ROMFAL_SHIFT)
#endif

	.text

	/* Values to program into memory controller registers */
tbl:	.long	MCCR1, MCCR1VAL
	.long	MCCR2, CONFIG_SYS_REFINT << MCCR2_REFINT_SHIFT
	.long	MCCR3
	.long	(((CONFIG_SYS_BSTOPRE & 0x000000f0) >> 4) << MCCR3_BSTOPRE2TO5_SHIFT) | \
		(CONFIG_SYS_REFREC << MCCR3_REFREC_SHIFT) | \
		(CONFIG_SYS_RDLAT  << MCCR3_RDLAT_SHIFT)
	.long	MCCR4
	.long	(CONFIG_SYS_PRETOACT << MCCR4_PRETOACT_SHIFT) | (CONFIG_SYS_ACTTOPRE << MCCR4_ACTTOPRE_SHIFT) | \
		(CONFIG_SYS_REGISTERD_TYPE_BUFFER << 20) | \
		(((CONFIG_SYS_BSTOPRE & 0x00000300) >> 8) << MCCR4_BSTOPRE0TO1_SHIFT ) | \
		((CONFIG_SYS_SDMODE_CAS_LAT << 4) | (CONFIG_SYS_SDMODE_WRAP << 3) | \
		(CONFIG_SYS_SDMODE_BURSTLEN) << MCCR4_SDMODE_SHIFT) | \
		(CONFIG_SYS_ACTTORW << MCCR4_ACTTORW_SHIFT) | \
		((CONFIG_SYS_BSTOPRE & 0x0000000f) << MCCR4_BSTOPRE6TO9_SHIFT )
	.long	MSAR1
	.long	(((CONFIG_SYS_BANK0_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK1_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK2_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK3_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 24)
	.long	EMSAR1
	.long	(((CONFIG_SYS_BANK0_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK1_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK2_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK3_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 24)
	.long	MSAR2
	.long	(((CONFIG_SYS_BANK4_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK5_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK6_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK7_START & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 24)
	.long	EMSAR2
	.long	(((CONFIG_SYS_BANK4_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK5_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK6_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK7_START & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 24)
	.long	MEAR1
	.long	(((CONFIG_SYS_BANK0_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK1_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK2_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK3_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 24)
	.long	EMEAR1
	.long	(((CONFIG_SYS_BANK0_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK1_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK2_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK3_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 24)
	.long	MEAR2
	.long	(((CONFIG_SYS_BANK4_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK5_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK6_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK7_END & MICR_ADDR_MASK) >> MICR_ADDR_SHIFT) << 24)
	.long	EMEAR2
	.long	(((CONFIG_SYS_BANK4_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  0) | \
		(((CONFIG_SYS_BANK5_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) <<  8) | \
		(((CONFIG_SYS_BANK6_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 16) | \
		(((CONFIG_SYS_BANK7_END & MICR_EADDR_MASK) >> MICR_EADDR_SHIFT) << 24)
	.long	0


	/*
	 * Early CPU initialization. Set up memory controller, so we can access any RAM at all. This
	 * must be done in assembly, since we have no stack at this point.
	 */
	.global	early_init_f
early_init_f:
	mflr	r10

	/* basic memory controller configuration */
	lis	r3, CONFIG_ADDR_HIGH
	lis	r4, CONFIG_DATA_HIGH
	bl	lab
lab:	mflr	r5
	lwzu	r0, tbl - lab(r5)
loop:	lwz	r1, 4(r5)
	stwbrx	r0, 0, r3
	eieio
	stwbrx	r1, 0, r4
	eieio
	lwzu	r0, 8(r5)
	cmpli	cr0, 0, r0, 0
	bne	cr0, loop

	/* set bank enable bits */
	lis	r0, MBER@h
	ori	r0, 0, MBER@l
	li	r1, CONFIG_SYS_BANK_ENABLE
	stwbrx	r0, 0, r3
	eieio
	stb	r1, 0(r4)
	eieio

	/* delay loop */
	lis	r0, 0x0003
	mtctr   r0
delay:	bdnz	delay

	/* enable memory controller */
	lis	r0, MCCR1@h
	ori	r0, 0, MCCR1@l
	stwbrx	r0, 0, r3
	eieio
	lwbrx	r0, 0, r4
	oris	r0, 0, MCCR1_MEMGO@h
	stwbrx	r0, 0, r4
	eieio

	/* set up stack pointer */
	lis	r1, CONFIG_SYS_INIT_SP_OFFSET@h
	ori	r1, r1, CONFIG_SYS_INIT_SP_OFFSET@l

	mtlr	r10
	blr

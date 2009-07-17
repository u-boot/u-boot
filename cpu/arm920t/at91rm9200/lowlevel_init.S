/*
 * Memory Setup stuff - taken from blob memsetup.S
 *
 * Copyright (C) 1999 2000 2001 Erik Mouw (J.A.K.Mouw@its.tudelft.nl) and
 *		       Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 * Modified for the at91rm9200dk board by
 * (C) Copyright 2004
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <version.h>

#ifndef CONFIG_SKIP_LOWLEVEL_INIT
/*
 * some parameters for the board
 *
 * This is based on rm9200dk.cfg for the BDI2000 from ABATRON which in
 * turn is based on the boot.bin code from ATMEL
 *
 */
#include <asm/arch/AT91RM9200.h>

_MTEXT_BASE:
#undef START_FROM_MEM
#ifdef START_FROM_MEM
	.word	TEXT_BASE-PHYS_FLASH_1
#else
	.word	TEXT_BASE
#endif

.globl lowlevel_init
lowlevel_init:
	/* Get the CKGR Base Address */
	ldr     r1, =AT91C_BASE_CKGR
	/* Main oscillator Enable register */
#ifdef CONFIG_SYS_USE_MAIN_OSCILLATOR
	ldr     r0, =0x0000FF01		/* Enable main oscillator,  OSCOUNT = 0xFF */
#else
	ldr     r0, =0x0000FF00		/* Disable main oscillator, OSCOUNT = 0xFF */
#endif
	str     r0, [r1, #AT91C_CKGR_MOR]
	/* Add loop to compensate Main Oscillator startup time */
	ldr     r0, =0x00000010
LoopOsc:
	subs    r0, r0, #1
	bhi     LoopOsc

	/* memory control configuration */
	/* this isn't very elegant, but	 what the heck */
	ldr	r0, =SMRDATA
	ldr	r1, _MTEXT_BASE
	sub	r0, r0, r1
	add	r2, r0, #80
0:
	/* the address */
	ldr	r1, [r0], #4
	/* the value */
	ldr	r3, [r0], #4
	str	r3, [r1]
	cmp	r2, r0
	bne	0b
	/* delay - this is all done by guess */
	ldr	r0, =0x00010000
	/* (vs reading PMC_SR for LOCKA, LOCKB ... or MOSCS earlier) */
1:
	subs	r0, r0, #1
	bhi	1b
	ldr	r0, =SMRDATA1
	ldr	r1, _MTEXT_BASE
	sub	r0, r0, r1
	add	r2, r0, #176
2:
	/* the address */
	ldr	r1, [r0], #4
	/* the value */
	ldr	r3, [r0], #4
	str	r3, [r1]
	cmp	r2, r0
	bne	2b

	/* switch from FastBus to Asynchronous clock mode */
	mrc	p15, 0, r0, c1, c0, 0
	orr	r0, r0, #0xC0000000	@ set bit 31 (iA) and 30 (nF)
	mcr	p15, 0, r0, c1, c0, 0

	/* everything is fine now */
	mov	pc, lr

	.ltorg

SMRDATA:
	.word AT91C_EBI_CFGR
	.word CONFIG_SYS_EBI_CFGR_VAL
	.word AT91C_SMC_CSR0
	.word CONFIG_SYS_SMC_CSR0_VAL
	.word AT91C_PLLAR
	.word CONFIG_SYS_PLLAR_VAL
	.word AT91C_PLLBR
	.word CONFIG_SYS_PLLBR_VAL
	.word AT91C_MCKR
	.word CONFIG_SYS_MCKR_VAL
	/* here there's a delay */
SMRDATA1:
	.word AT91C_PIOC_ASR
	.word CONFIG_SYS_PIOC_ASR_VAL
	.word AT91C_PIOC_BSR
	.word CONFIG_SYS_PIOC_BSR_VAL
	.word AT91C_PIOC_PDR
	.word CONFIG_SYS_PIOC_PDR_VAL
	.word AT91C_EBI_CSA
	.word CONFIG_SYS_EBI_CSA_VAL
	.word AT91C_SDRC_CR
	.word CONFIG_SYS_SDRC_CR_VAL
	.word AT91C_SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91C_SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL1
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91C_SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL2
	.word CONFIG_SYS_SDRAM1
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91C_SDRC_TR
	.word CONFIG_SYS_SDRC_TR_VAL
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	.word AT91C_SDRC_MR
	.word CONFIG_SYS_SDRC_MR_VAL3
	.word CONFIG_SYS_SDRAM
	.word CONFIG_SYS_SDRAM_VAL
	/* SMRDATA1 is 176 bytes long */
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

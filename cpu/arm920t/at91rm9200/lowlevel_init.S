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

/* flash */
#define MC_PUIA		0xFFFFFF10
#define MC_PUP		0xFFFFFF50
#define MC_PUER		0xFFFFFF54
#define MC_ASR		0xFFFFFF04
#define MC_AASR		0xFFFFFF08
#define EBI_CFGR	0xFFFFFF64
#define SMC_CSR0	0xFFFFFF70

/* clocks */
#define PLLAR		0xFFFFFC28
#define PLLBR		0xFFFFFC2C
#define MCKR		0xFFFFFC30

#define AT91C_BASE_CKGR 0xFFFFFC20
#define CKGR_MOR 0

/* sdram */
#define PIOC_ASR	0xFFFFF870
#define PIOC_BSR	0xFFFFF874
#define PIOC_PDR	0xFFFFF804
#define EBI_CSA		0xFFFFFF60
#define SDRC_CR		0xFFFFFF98
#define SDRC_MR		0xFFFFFF90
#define SDRC_TR		0xFFFFFF94


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
#ifdef CFG_USE_MAIN_OSCILLATOR
	ldr     r0, =0x0000FF01		/* Enable main oscillator,  OSCOUNT = 0xFF */
#else
	ldr     r0, =0x0000FF00		/* Disable main oscillator, OSCOUNT = 0xFF */
#endif
	str     r0, [r1, #CKGR_MOR]
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
	.word MC_PUIA
	.word MC_PUIA_VAL
	.word MC_PUP
	.word MC_PUP_VAL
	.word MC_PUER
	.word MC_PUER_VAL
	.word MC_ASR
	.word MC_ASR_VAL
	.word MC_AASR
	.word MC_AASR_VAL
	.word EBI_CFGR
	.word EBI_CFGR_VAL
	.word SMC_CSR0
	.word SMC_CSR0_VAL
	.word PLLAR
	.word PLLAR_VAL
	.word PLLBR
	.word PLLBR_VAL
	.word MCKR
	.word MCKR_VAL
	/* SMRDATA is 80 bytes long */
	/* here there's a delay of 100 */
SMRDATA1:
	.word PIOC_ASR
	.word PIOC_ASR_VAL
	.word PIOC_BSR
	.word PIOC_BSR_VAL
	.word PIOC_PDR
	.word PIOC_PDR_VAL
	.word EBI_CSA
	.word EBI_CSA_VAL
	.word SDRC_CR
	.word SDRC_CR_VAL
	.word SDRC_MR
	.word SDRC_MR_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRC_MR
	.word SDRC_MR_VAL1
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRC_MR
	.word SDRC_MR_VAL2
	.word SDRAM1
	.word SDRAM_VAL
	.word SDRC_TR
	.word SDRC_TR_VAL
	.word SDRAM
	.word SDRAM_VAL
	.word SDRC_MR
	.word SDRC_MR_VAL3
	.word SDRAM
	.word SDRAM_VAL
	/* SMRDATA1 is 176 bytes long */
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

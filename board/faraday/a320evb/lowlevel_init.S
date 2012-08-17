/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <config.h>
#include <version.h>

#include <asm/macro.h>
#include <faraday/ftsdmc020.h>

/*
 * parameters for the SDRAM controller
 */
#define TP0_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_TP0)
#define TP1_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_TP1)
#define CR_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_CR)
#define B0_BSR_A	(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_BANK0_BSR)
#define ACR_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_ACR)

#define TP0_D		CONFIG_SYS_FTSDMC020_TP0
#define TP1_D		CONFIG_SYS_FTSDMC020_TP1
#define CR_D1		FTSDMC020_CR_IPREC
#define CR_D2		FTSDMC020_CR_ISMR
#define CR_D3		FTSDMC020_CR_IREF

#define B0_BSR_D	(CONFIG_SYS_FTSDMC020_BANK0_BSR | \
			FTSDMC020_BANK_BASE(PHYS_SDRAM_1))
#define ACR_D		FTSDMC020_ACR_TOC(0x18)

/*
 * numeric 7 segment display
 */
.macro	led, num
	write32	CONFIG_DEBUG_LED, \num
.endm

/*
 * Waiting for SDRAM to set up
 */
.macro	wait_sdram
	ldr	r0, =CONFIG_FTSDMC020_BASE
1:
	ldr	r1, [r0, #FTSDMC020_OFFSET_CR]
	cmp	r1, #0
	bne	1b
.endm

.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

	led	0x0

	bl	init_sdmc

	led	0x1

	/* everything is fine now */
	mov	lr, r11
	mov	pc, lr

/*
 * memory initialization
 */
init_sdmc:
	led	0x10

	/* set SDRAM register */

	write32	TP0_A, TP0_D
	led	0x11

	write32	TP1_A, TP1_D
	led	0x12

	/* set to precharge */
	write32	CR_A, CR_D1
	led	0x13

	wait_sdram
	led	0x14

	/* set mode register */
	write32	CR_A, CR_D2
	led	0x15

	wait_sdram
	led	0x16

	/* set to refresh */
	write32	CR_A, CR_D3
	led	0x17

	wait_sdram
	led	0x18

	write32	B0_BSR_A, B0_BSR_D
	led	0x19

	write32	ACR_A, ACR_D
	led	0x1a

	mov	pc, lr

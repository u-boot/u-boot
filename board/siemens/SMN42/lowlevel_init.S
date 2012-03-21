/*
 * (C) Copyright 2006 Embedded Artists AB <www.embeddedartists.com>
 *
 * Slight modifications made to support the SMN42 board from Siemens.
 * 2007 Gary Jennejohn garyj@denx.de
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
#include <version.h>
#include <asm/arch/hardware.h>

/* some parameters for the board */
/* setting up the CPU-internal memory */
#define	SRAM_START	0x40000000
#define	SRAM_SIZE	0x00004000
#define BCFG0_VALUE	0x1000ffef
#define BCFG1_VALUE	0x10005D2F
#define BCFG2_VALUE	0x10005D2F
/*
 * For P0.18 to set ZZ to the SRAMs to 1. Also set P0.2 (SCL) and P0.3 (SDA)
 * for the bit-banger I2C driver correctly.
 */
#define   IO0_VALUE   0x4000C

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE
MEMMAP_ADR:
	.word	MEMMAP
BCFG0_ADR:
	.word BCFG0
_BCFG0_VALUE:
	.word BCFG0_VALUE
BCFG1_ADR:
	.word	BCFG1
_BCFG1_VALUE:
	.word	BCFG1_VALUE
BCFG2_ADR:
	.word	BCFG2
_BCFG2_VALUE:
	.word	BCFG2_VALUE
IO0DIR_ADR:
	.word	IO0DIR
_IO0DIR_VALUE:
	.word	IO0_VALUE
IO0SET_ADR:
	.word	IO0SET
_IO0SET_VALUE:
	.word	IO0_VALUE
PINSEL2_ADR:
	.word	PINSEL2
PINSEL2_MASK:
	.word	0x00000000
PINSEL2_VALUE:
	.word	0x0F804914

.extern _start

.globl lowlevel_init
lowlevel_init:
	/* set up memory control register for bank 0 */
	ldr r0, _BCFG0_VALUE
	ldr r1, BCFG0_ADR
	str r0, [r1]

	/* set up memory control register for bank 1 */
	ldr	r0, _BCFG1_VALUE
	ldr	r1, BCFG1_ADR
	str	r0, [r1]

	/* set up memory control register for bank 2 */
	ldr	r0, _BCFG2_VALUE
	ldr	r1, BCFG2_ADR
	str	r0, [r1]

	/* set IO0DIR to make P0.2, P0.3  and P0.18 outputs */
	ldr	r0, _IO0DIR_VALUE
	ldr	r1, IO0DIR_ADR
	str	r0, [r1]

	/* set P0.18 to 1 */
	ldr	r0, _IO0SET_VALUE
	ldr	r1, IO0SET_ADR
	str	r0, [r1]

	/* set up PINSEL2 for bus-pins */
	ldr	r0, PINSEL2_ADR
	ldr	r1, [r0]
	ldr	r2, PINSEL2_MASK
	ldr	r3, PINSEL2_VALUE
	and	r1, r1, r2
	orr	r1, r1, r3
	str	r1, [r0]

	/* move vectors to beginning of SRAM */
	mov	r2, #SRAM_START
	mov	r0, #0 /*_start*/
	ldmneia r0!, {r3-r10}
	stmneia r2!, {r3-r10}
	ldmneia r0, {r3-r9}
	stmneia r2, {r3-r9}

	/* Set-up MEMMAP register, so vectors are taken from SRAM */
	ldr	r0, MEMMAP_ADR
	mov	r1, #0x02	/* vectors re-mapped to static RAM */
	str	r1, [r0]

	/* everything is fine now */
	mov	pc, lr

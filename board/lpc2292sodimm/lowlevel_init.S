/*
 * (C) Copyright 2006 Embedded Artists AB <www.embeddedartists.com>
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
/* setting up the memory */
#define	SRAM_START	0x40000000
#define	SRAM_SIZE	0x00004000
#define BCFG0_VALUE	0x1000ffef
#define BCFG1_VALUE	0x10001C61

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE
MEMMAP_ADR:
	.word	MEMMAP
BCFG0_ADR:
	.word	BCFG0
_BCFG0_VALUE:
	.word	BCFG0_VALUE
BCFG1_ADR:
	.word	BCFG1
_BCFG1_VALUE:
	.word	BCFG1_VALUE
PINSEL2_ADR:
	.word	PINSEL2
PINSEL2_MASK:
	.word	0x00000000
PINSEL2_VALUE:
	.word	0x0F800914

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

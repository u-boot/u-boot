/*
 * lowlevel_init.S
 *
 * AM33XX low level initialization.
 *
 * Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
 *
 * Initial Code by:
 * Mansoor Ahamed  <mansoor.ahamed@ti.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <config.h>
#include <asm/arch/hardware.h>

_mark1:
	.word mark1
_lowlevel_init1:
	.word lowlevel_init
_s_init_start:
	.word s_init_start

_TEXT_BASE:
	.word	CONFIG_SYS_TEXT_BASE	/* sdram load addr from config.mk */

/*****************************************************************************
 * lowlevel_init: - Platform low level init.
 ****************************************************************************/
.globl lowlevel_init
lowlevel_init:

	/* The link register is saved in ip by start.S */
	mov r6, ip
	/* check if we are already running from RAM */
	ldr r2, _lowlevel_init1
	ldr r3, _TEXT_BASE
	sub r4, r2, r3
	sub r0, pc, r4
	ldr sp, SRAM_STACK
mark1:
	ldr r5, _mark1
	sub r5, r5, r2 /* bytes between mark1 and lowlevel_init */
	sub r0, r0, r5 /* r0 <- _start w.r.t current place of execution */
	mov r10, #0x0 /* r10 has in_ddr used by s_init() */

	ands r0, r0, #0xC0000000
	/* MSB 2 bits <> 0 then we are in ocmc or DDR */
	cmp r0, #0x80000000
	bne s_init_start
	mov r10, #0x01
	b s_init_start

s_init_start:
	mov r0, r10 /* passing in_ddr in r0 */
	bl s_init
	/* back to arch calling code */
	mov pc, r6
	/* the literal pools origin */
	.ltorg

SRAM_STACK:
	/* Place stack at the top */
	.word LOW_LEVEL_SRAM_STACK

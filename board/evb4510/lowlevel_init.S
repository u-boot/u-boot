/*
 * Copyright (c) 2004	Cucy Systems (http://www.cucy.com)
 * Curt Brune <curt@cucy.com>
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


#include <config.h>
#include <version.h>
#include <asm/hardware.h>

/***********************************************************************
 * Configure Memory Map
 *
 * This memory map allows us to relocate from FLASH to SRAM.  After
 * power-on reset the CPU only knows about the FLASH memory at address
 * 0x00000000.	After lowlevel_init completes the memory map will be:
 *
 * Memory Addr
 * 0x00000000
 *     to	8MB SRAM (U5) -- 8MB Map
 * 0x00800000
 *
 * 0x01000000
 *     to	2MB Flash @ 0x00000000 (U7) -- 2MB Map
 * 0x01200000
 *
 * 0x02000000
 *     to	512KB Flash @ 0x02000000 (U9) -- 2MB Map
 * 0x02080000
 *
 * Load all 12 memory registers with the STMIA instruction since
 * memory access is disabled once these registers are written.	The
 * last register written re-enables memory access.  For more info see
 * the user's manual for the S3C4510B, available from Samsung's web
 * site.  Search for part number "S3C4510B".
 *
 ***********************************************************************/

.globl lowlevel_init
lowlevel_init:

	/* preserve the temp register (r12 AKA ip) and remap it. */
	ldr r1, =SRAM_BASE+0xC
	add r0, r12, #0x01000000
	str r0, [r1]

	/* remap the link register for when we return */
	add lr, lr, #0x01000000

	/* store a short program in the on chip SRAM, which is
	 * unaffected when remapping memory.  Note the cache must be
	 * disabled for the on chip SRAM to be available.
	 */
	ldr r1, =SRAM_BASE
	ldr r0, =0xe8801ffe  /* stmia r0, {r1-r12} */
	str r0, [r1]
	add r1, r1, #4
	ldr r0, =0xe59fc000  /* ldr r12, [pc, #0]  */
	str r0, [r1]
	add r1, r1, #4
	ldr r0, =0xe1a0f00e  /* mov pc, lr	   */
	str r0, [r1]

	adr   r0, memory_map_data
	ldmia r0, {r1-r12}
	ldr   r0, =REG_EXTDBWTH

	ldr pc, =SRAM_BASE

.globl reset_cpu
reset_cpu:
	/*
	 * reset the cpu by re-mapping FLASH 0 to 0x0 and jumping to
	 * address 0x0.	 We accomplish this by storing a few
	 * instructions into the on chip SRAM (8KB) and run from
	 * there.  Note the cache must be disabled for the on chip
	 * SRAM to be available.
	 *
	 * load r2 with REG_ROMCON0
	 * load r3 with 0x12040060	   configure FLASH bank 0 @ 0x00000000
	 * load r4 with REG_DRAMCON0
	 * load r5 with 0x08000380	   configure   RAM bank 0 @ 0x01000000
	 * load r6 with REG_REFEXTCON
	 * load r7 with 0x9c218360
	 * load r8 with 0x0
	 * store str r3,[r2] @ SRAM_BASE
	 * store str r5,[r4] @ SRAM_BASE + 0x4
	 * store str r7,[r6] @ SRAM_BASE + 0x8
	 * store mov pc,r8   @ SRAM_BASE + 0xC
	 * mov pc, SRAM_BASE
	 *
	 */

	/* disable cache */
	ldr r0, =REG_SYSCFG
	ldr r1, =0x83ffffa0	/* cache-disabled  */
	str r1, [r0]

	ldr r2, =REG_ROMCON0
	ldr r3, =0x02000060	/* Bank0 2MB FLASH   @ 0x00000000 */
	ldr r4, =REG_DRAMCON0
	ldr r5, =0x18040380	/* DRAM0 8MB SRAM    @ 0x01000000 */
	ldr r6, =REG_REFEXTCON
	ldr r7, =0xce278360
	ldr r8, =0x00000000
	ldr r1, =SRAM_BASE
	ldr r0, =0xe5823000	/* str	r3, [r2] */
	str r0, [r1]
	ldr r1, =SRAM_BASE+4
	ldr r0, =0xe5845000	/* str	r5, [r4] */
	str r0, [r1]
	ldr r1, =SRAM_BASE+8
	ldr r0, =0xe5867000	/* str	r7, [r6] */
	str r0, [r1]
	ldr r1, =SRAM_BASE+0xC
	ldr r0, =0xe1a0f008	/* mov	pc, r8	 */
	str r0, [r1]
	ldr r1, =SRAM_BASE
	mov pc, r1

	/* never return */

/************************************************************************
 * Below are twelve 32-bit values for the twelve memory registers of
 * the system manager, starting with register REG_EXTDBWTH.
 ***********************************************************************/
memory_map_data:
	.long 0x00f03005     /* memory widths */
	.long 0x12040060     /* Bank0 2MB FLASH	  @ 0x01000000 */
	.long 0x22080060     /* Bank1 512KB FLASH @ 0x02000000 */
	.long 0x00000000
	.long 0x00000000
	.long 0x00000000
	.long 0x00000000
	.long 0x08000380     /* DRAM0 8MB SRAM	  @ 0x00000000 */
	.long 0x00000000
	.long 0x00000000
	.long 0x00000000
	.long 0x9c218360     /* enable memory */

/*
 * (C) Copyright 2009 DENX Software Engineering
 * Author: John Rigby <jrigby@gmail.com>
 *
 * Based on U-Boot and RedBoot sources for several different i.mx
 * platforms.
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

#include <asm/macro.h>
#include <asm/arch/macro.h>

.macro init_clocks
	/*
	 * clocks
	 *
	 * first enable CLKO debug output
	 * 0x40000000 enables the debug CLKO signal
	 * 0x05000000 sets CLKO divider to 6
	 * 0x00600000 makes CLKO parent clk the USB clk
	 */
	write32	0x53f80064, 0x45600000

	/* CCTL: ARM = 399 MHz, AHB = 133 MHz */
	write32	0x53f80008, 0x20034000

	/*
	 * PCDR2: NFC = 33.25 MHz
	 * This is required for the NAND Flash of this board, which is a Samsung
	 * K9F1G08U0B with 25-ns R/W cycle times, in order to make it work with
	 * the NFC driver in symmetric (i.e. one-cycle) mode.
	 */
	write32	0x53f80020, 0x01010103

	/*
	 * enable all implemented clocks in all three
	 * clock control registers
	 */
	write32	0x53f8000c, 0x1fffffff
	write32	0x53f80010, 0xffffffff
	write32	0x53f80014, 0xfdfff
.endm

.macro init_ddrtype
	/*
	 * ddr_type is 3.3v SDRAM
	 */
	write32	0x43fac454, 0x800
.endm

/*
 * sdram controller init
 */
.macro init_sdram_bank bankaddr, ctl, cfg
	ldr	r0, =0xb8001000
	ldr	r2, =\bankaddr
	/*
	 * reset SDRAM controller
	 * then wait for initialization to complete
	 */
	ldr	r1, =(1 << 1)
	str	r1, [r0, #0x10]
1:	ldr	r3, [r0, #0x10]
	tst	r3, #(1 << 31)
	beq	1b

	ldr	r1, =0x95728
	str	r1, [r0, #\cfg]		/* config */

	ldr	r1, =0x92116480		/* control | precharge */
	str	r1, [r0, #\ctl]		/* write command to controller */
	str	r1, [r2, #0x400]	/* command encoded in address */

	ldr	r1, =0xa2116480		/* auto refresh */
	str	r1, [r0, #\ctl]
	ldrb	r3, [r2]		/* read dram twice to auto refresh */
	ldrb	r3, [r2]

	ldr	r1, =0xb2116480		/* control | load mode */
	str	r1, [r0, #\ctl]		/* write command to controller */
	strb	r1, [r2, #0x33]		/* command encoded in address */

	ldr	r1, =0x82116480		/* control  | normal (0)*/
	str	r1, [r0, #\ctl]		/* write command to controller */
.endm

.globl lowlevel_init
lowlevel_init:
	init_aips
	init_max
	init_m3if
	init_clocks

	init_sdram_bank 0x80000000, 0x0, 0x4

	init_sdram_bank 0x90000000, 0x8, 0xc
	mov	pc, lr

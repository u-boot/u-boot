/*
 *  U-boot - i386 Startup Code
 *
 *  Copyright (c) 2002, 2003 Omicron Ceti AB, Daniel Engström <denaiel@omicron.se>
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


#define BOOT_SEG	0xffff0000	/* linear segment of boot code */
#define a32		.byte 0x67;
#define o32		.byte 0x66;

.section .start16, "ax"
.code16
.globl start16
start16:
	/* First we let the BSP do some early initialization
	 * this code have to map the flash to its final position
	 */
	mov	$board_init16_ret, %bp
	jmp	board_init16
board_init16_ret:

	/* Turn of cache (this might require a 486-class CPU) */
	movl    %cr0, %eax
	orl     $0x60000000,%eax
	movl    %eax, %cr0
	wbinvd

	/* load the descriptor tables */
o32 cs	lidt	idt_ptr
o32 cs	lgdt    gdt_ptr


	/* Now, we enter protected mode */
	movl    %cr0, %eax
	orl     $1,%eax
	movl    %eax, %cr0

	/* Flush the prefetch queue */
	jmp     ff
ff:

	/* Finally jump to the 32bit initialization code */
	movw	$code32start, %ax
	movw    %ax,%bp
o32 cs	ljmp	*(%bp)

	/* 48-bit far pointer */
code32start:
	.long	_start                          /* offset */
	.word	0x10                            /* segment */

idt_ptr:
	.word	0				/* limit */
	.long	0		       		/* base */

gdt_ptr:
	.word	0x30			        /* limit (48 bytes = 6 GDT entries) */
	.long	BOOT_SEG + gdt	        	/* base */

	/* The GDT table ...
	 *
	 *       Selector	Type
	 * 	 0x00		NULL
	 * 	 0x08		Unused
	 *	 0x10		32bit code
	 *	 0x18		32bit data/stack
	 *	 0x20		16bit code
	 *	 0x28	 	16bit data/stack
	 */

gdt:
	.word	0, 0, 0, 0			/* NULL  */
	.word	0, 0, 0, 0			/* unused */

	.word	0xFFFF				/* 4Gb - (0x100000*0x1000 = 4Gb) */
	.word	0				/* base address = 0 */
	.word	0x9B00				/* code read/exec */
	.word	0x00CF				/* granularity = 4096, 386 (+5th nibble of limit) */

	.word	0xFFFF				/* 4Gb - (0x100000*0x1000 = 4Gb) */
	.word	0x0				/* base address = 0 */
	.word	0x9300				/* data read/write */
	.word	0x00CF				/* granularity = 4096, 386 (+5th nibble of limit) */

	.word	0xFFFF				/* 64kb */
	.word	0				/* base address = 0 */
	.word	0x9b00				/* data read/write */
	.word	0x0010				/* granularity = 1  (+5th nibble of limit) */

	.word	0xFFFF				/* 64kb */
	.word	0				/* base address = 0 */
	.word	0x9300				/* data read/write */
	.word	0x0010				/* granularity = 1 (+5th nibble of limit) */

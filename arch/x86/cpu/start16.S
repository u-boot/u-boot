/*
 *  U-boot - x86 Startup Code
 *
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002,2003
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
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

#include <asm/global_data.h>
#include <asm/processor-flags.h>

#define BOOT_SEG	0xffff0000	/* linear segment of boot code */
#define a32		.byte 0x67;
#define o32		.byte 0x66;

.section .start16, "ax"
.code16
.globl start16
start16:
	/*
	 * First we let the BSP do some early initialization
	 * this code have to map the flash to its final position
	 */
	jmp	board_init16
.globl board_init16_ret
board_init16_ret:

	/* Turn of cache (this might require a 486-class CPU) */
	movl	%cr0, %eax
	orl	$(X86_CR0_NW | X86_CR0_CD), %eax
	movl	%eax, %cr0
	wbinvd

	/* load the temporary Global Descriptor Table */
o32 cs	lidt	idt_ptr
o32 cs	lgdt	gdt_ptr

	/* Now, we enter protected mode */
	movl	%cr0, %eax
	orl	$X86_CR0_PE, %eax
	movl	%eax, %cr0

	/* Flush the prefetch queue */
	jmp	ff
ff:
	/* Finally jump to the 32bit initialization code */
	movw	$code32start, %ax
	movw	%ax, %bp
o32 cs	ljmp	*(%bp)

	/* 48-bit far pointer */
code32start:
	.long	_start		/* offset */
	.word	0x10		/* segment */

idt_ptr:
	.word	0		/* limit */
	.long	0		/* base */

/*
 * The following Global Descriptor Table is just enough to get us into
 * 'Flat Protected Mode' - It will be discarded as soon as the final
 * GDT is setup in a safe location in RAM
 */
gdt_ptr:
	.word	0x20		/* limit (32 bytes = 4 GDT entries) */
	.long	BOOT_SEG + gdt	/* base */

/* Some CPUs are picky about GDT alignment... */
.align 16
gdt:
	/*
	 * The GDT table ...
	 *
	 *	 Selector	Type
	 *	 0x00		NULL
	 *	 0x08		Unused
	 *	 0x10		32bit code
	 *	 0x18		32bit data/stack
	 */
	/* The NULL Desciptor - Mandatory */
	.word	0x0000		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x00		/* access */
	.byte	0x00		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/* Unused Desciptor - (matches Linux) */
	.word	0x0000		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x00		/* access */
	.byte	0x00		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/*
	 * The Code Segment Descriptor:
	 * - Base   = 0x00000000
	 * - Size   = 4GB
	 * - Access = Present, Ring 0, Exec (Code), Readable
	 * - Flags  = 4kB Granularity, 32-bit
	 */
	.word	0xffff		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x9b		/* access */
	.byte	0xcf		/* flags + limit_high */
	.byte	0x00		/* base_high */

	/*
	 * The Data Segment Descriptor:
	 * - Base   = 0x00000000
	 * - Size   = 4GB
	 * - Access = Present, Ring 0, Non-Exec (Data), Writable
	 * - Flags  = 4kB Granularity, 32-bit
	 */
	.word	0xffff		/* limit_low */
	.word	0x0000		/* base_low */
	.byte	0x00		/* base_middle */
	.byte	0x93		/* access */
	.byte	0xcf		/* flags + limit_high */
	.byte	0x00		/* base_high */

/*
 * (C) Copyright 2002
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

/*
 * Based on msbios.c from rolo 1.6:
 *----------------------------------------------------------------------
 * (C) Copyright 2000
 * Sysgo Real-Time Solutions GmbH
 * Klein-Winternheim, Germany
 *----------------------------------------------------------------------
 */

#include "bios.h"

/*
 * During it's initialization phase, before switching to protected
 * mode, the Linux Kernel makes a few BIOS calls. This won't work
 * if the board does not have a BIOS.
 *
 * This is a very minimalisic BIOS that supplies just enough
 * functionality to keep the Linux Kernel happy. It is NOT
 * a general purpose replacement for a real BIOS !!
 */

.section .bios, "ax"
.code16
.org 0
	/* a call to f000:0 should warmboot */
	jmp	realmode_reset

.globl rm_int00
.hidden rm_int00
.type rm_int00, @function
rm_int00:
	pushw	$0
	jmp	any_interrupt16
.globl rm_int01
.hidden rm_int01
.type rm_int01, @function
rm_int01:
	pushw	$1
	jmp	any_interrupt16
.globl rm_int02
.hidden rm_int02
.type rm_int02, @function
rm_int02:
	pushw	$2
	jmp	any_interrupt16
.globl rm_int03
.hidden rm_int03
.type rm_int03, @function
rm_int03:
	pushw	$3
	jmp	any_interrupt16
.globl rm_int04
.hidden rm_int04
.type rm_int04, @function
rm_int04:
	pushw	$4
	jmp	any_interrupt16
.globl rm_int05
.hidden rm_int05
.type rm_int05, @function
rm_int05:
	pushw	$5
	jmp	any_interrupt16
.globl rm_int06
.hidden rm_int06
.type rm_int06, @function
rm_int06:
	pushw	$6
	jmp	any_interrupt16
.globl rm_int07
.hidden rm_int07
.type rm_int07, @function
rm_int07:
	pushw	$7
	jmp	any_interrupt16
.globl rm_int08
.hidden rm_int08
.type rm_int08, @function
rm_int08:
	pushw	$8
	jmp	any_interrupt16
.globl rm_int09
.hidden rm_int09
.type rm_int09, @function
rm_int09:
	pushw	$9
	jmp	any_interrupt16
.globl rm_int0a
.hidden rm_int0a
.type rm_int0a, @function
rm_int0a:
	pushw	$10
	jmp	any_interrupt16
.globl rm_int0b
.hidden rm_int0b
.type rm_int0b, @function
rm_int0b:
	pushw	$11
	jmp	any_interrupt16
.globl rm_int0c
.hidden rm_int0c
.type rm_int0c, @function
rm_int0c:
	pushw	$12
	jmp	any_interrupt16
.globl rm_int0d
.hidden rm_int0d
.type rm_int0d, @function
rm_int0d:
	pushw	$13
	jmp	any_interrupt16
.globl rm_int0e
.hidden rm_int0e
.type rm_int0e, @function
rm_int0e:
	pushw	$14
	jmp	any_interrupt16
.globl rm_int0f
.hidden rm_int0f
.type rm_int0f, @function
rm_int0f:
	pushw	$15
	jmp	any_interrupt16
.globl rm_int10
.hidden rm_int10
.type rm_int10, @function
rm_int10:
	pushw	$16
	jmp	any_interrupt16
.globl rm_int11
.hidden rm_int11
.type rm_int11, @function
rm_int11:
	pushw	$17
	jmp	any_interrupt16
.globl rm_int12
.hidden rm_int12
.type rm_int12, @function
rm_int12:
	pushw	$18
	jmp	any_interrupt16
.globl rm_int13
.hidden rm_int13
.type rm_int13, @function
rm_int13:
	pushw	$19
	jmp	any_interrupt16
.globl rm_int14
.hidden rm_int14
.type rm_int14, @function
rm_int14:
	pushw	$20
	jmp	any_interrupt16
.globl rm_int15
.hidden rm_int15
.type rm_int15, @function
rm_int15:
	pushw	$21
	jmp	any_interrupt16
.globl rm_int16
.hidden rm_int16
.type rm_int16, @function
rm_int16:
	pushw	$22
	jmp	any_interrupt16
.globl rm_int17
.hidden rm_int17
.type rm_int17, @function
rm_int17:
	pushw	$23
	jmp	any_interrupt16
.globl rm_int18
.hidden rm_int18
.type rm_int18, @function
rm_int18:
	pushw	$24
	jmp	any_interrupt16
.globl rm_int19
.hidden rm_int19
.type rm_int19, @function
rm_int19:
	pushw	$25
	jmp	any_interrupt16
.globl rm_int1a
.hidden rm_int1a
.type rm_int1a, @function
rm_int1a:
	pushw	$26
	jmp	any_interrupt16
.globl rm_int1b
.hidden rm_int1b
.type rm_int1b, @function
rm_int1b:
	pushw	$27
	jmp	any_interrupt16
.globl rm_int1c
.hidden rm_int1c
.type rm_int1c, @function
rm_int1c:
	pushw	$28
	jmp	any_interrupt16
.globl rm_int1d
.hidden rm_int1d
.type rm_int1d, @function
rm_int1d:
	pushw	$29
	jmp	any_interrupt16
.globl rm_int1e
.hidden rm_int1e
.type rm_int1e, @function
rm_int1e:
	pushw	$30
	jmp	any_interrupt16
.globl rm_int1f
.hidden rm_int1f
.type rm_int1f, @function
rm_int1f:
	pushw	$31
	jmp	any_interrupt16
.globl rm_def_int
.hidden rm_def_int
.type rm_def_int, @function
rm_def_int:
	iret


	/*
	 * All interrupt jumptable entries jump to here
	 * after pushing the interrupt vector number onto the
	 * stack.
	 */
any_interrupt16:
	MAKE_BIOS_STACK

gs	movw	OFFS_VECTOR(%bp), %ax
	cmpw	$0x10, %ax
	je	Lint_10h
	cmpw	$0x11, %ax
	je	Lint_11h
	cmpw	$0x12, %ax
	je	Lint_12h
	cmpw	$0x13, %ax
	je	Lint_13h
	cmpw	$0x15, %ax
	je	Lint_15h
	cmpw	$0x16, %ax
	je	Lint_16h
	cmpw	$0x1a, %ax
	je	Lint_1ah
	movw	$0xffff, %ax
	jmp	Lout
Lint_10h:					/* VGA BIOS services */
	call	bios_10h
	jmp	Lout
Lint_11h:
	call	bios_11h
	jmp	Lout
Lint_12h:
	call	bios_12h
	jmp	Lout
Lint_13h:					/* BIOS disk services */
	call	bios_13h
	jmp	Lout
Lint_15h:					/* Misc. BIOS services */
	call	bios_15h
	jmp	Lout
Lint_16h:					/* keyboard services */
	call	bios_16h
	jmp	Lout
Lint_1ah:					/* PCI bios */
	call	bios_1ah
	jmp	Lout
Lout:
	cmpw	$0, %ax
	je	Lhandeled

	/* Insert code for unhandeled INTs here.
	 *
	 * ROLO prints a message to the console
	 * (we could do that but then we're in 16bit mode
	 * so we'll have to get back into 32bit mode
	 * to use the console I/O routines (if we do this
	 * we shuls make int 0x10 and int 0x16 work as well))
	 */
Lhandeled:
	RESTORE_CALLERS_STACK
	addw	$2,%sp				/* dump vector number */
	iret					/* return from interrupt */


/*
 ************************************************************
 * BIOS	interrupt 10h -- VGA services
 ************************************************************
 */
bios_10h:
gs	movw	OFFS_AX(%bp), %ax
	shrw	$8, %ax
	cmpw	$0x3, %ax
	je	Lcur_pos
	cmpw	$0xf, %ax
	je	Lvid_state
	cmpw	$0x12, %ax
	je	Lvid_cfg
	movw	$0xffff, %ax
	ret
Lcur_pos:					/* Read Cursor Position and Size */
gs	movw	$0, OFFS_CX(%bp)
gs	movw	$0, OFFS_DX(%bp)
	xorw	%ax, %ax
	ret
Lvid_state:					/* Get Video State */
gs	movw	$(80 << 8|0x03), OFFS_AX(%bp)	/* 80 columns, 80x25, 16 colors */
gs	movw	$0, OFFS_BX(%bp)
	xorw	%ax, %ax
	ret
Lvid_cfg:	/* Video Subsystem Configuration (EGA/VGA) */
gs	movw	$0x10, OFFS_BX(%bp)		/* indicate CGA/MDA/HGA */
	xorw	%ax, %ax
	ret


/*
 ************************************************************
 * BIOS interrupt 11h -- Equipment determination
 ************************************************************
 */

bios_11h:
cs	movw	bios_equipment, %ax
gs	movw	%ax, OFFS_AX(%bp)
	xorw	%ax, %ax
	ret


/*
 ************************************************************
 * BIOS	interrupt 12h -- Get Memory Size
 ************************************************************
 */
bios_12h:
cs	movw	ram_in_64kb_chunks, %ax
	cmpw	$0xa, %ax
	ja	b12_more_than_640k
	shlw	$6, %ax
	jmp	b12_return
b12_more_than_640k:
	movw	$0x280, %ax
b12_return:
gs	movw	%ax, OFFS_AX(%bp)		/* return number of kilobytes in ax */

gs	movw	OFFS_FLAGS(%bp), %ax
	andw	$0xfffe, %ax			/* clear carry -- function succeeded */
gs	movw	%ax, OFFS_FLAGS(%bp)

	xorw	%ax, %ax
	ret


/*
 ************************************************************
 * BIOS interrupt 13h -- Disk services
 ************************************************************
 */
bios_13h:
gs	movw	OFFS_AX(%bp), %ax
	shrw	$8, %ax
	cmpw	$0x15, %ax
	je	Lfunc_15h
	movw	$0xffff, %ax
	ret
Lfunc_15h:
gs	movw	OFFS_AX(%bp), %ax
	andw	$0xff, %ax			/* return AH=0->drive not present */
gs	movw	%ax, OFFS_AX(%bp)
	xorw	%ax, %ax
	ret


/*
 ***********************************************************
 * BIOS interrupt 15h -- Miscellaneous services
 ***********************************************************
 */
bios_15h:
gs	movw	OFFS_AX(%bp), %ax
	shrw	$8, %ax
	cmpw	$0xc0, %ax
	je	Lfunc_c0h
	cmpw	$0xe8, %ax
	je	Lfunc_e8h
	cmpw	$0x88, %ax
	je	Lfunc_88h
	movw	$0xffff, %ax
	ret

Lfunc_c0h:					/* Return System Configuration Parameters (PS2 only) */
gs	movw	OFFS_FLAGS(%bp), %ax
	orw	$1, %ax				/* return carry -- function not supported */
gs	movw	%ax, OFFS_FLAGS(%bp)
	xorw	%ax, %ax
	ret

Lfunc_e8h:
gs	movw	OFFS_AX(%bp), %ax
	andw	$0xff, %ax
	cmpw	$1, %ax
	je	Lfunc_e801h
gs	movw	OFFS_FLAGS(%bp), %ax
	orw	$1, %ax				/* return carry -- function not supported */
gs	movw	%ax, OFFS_FLAGS(%bp)
	xorw	%ax, %ax
	ret

Lfunc_e801h:					/* Get memory size for >64M Configurations */
cs	movw	ram_in_64kb_chunks, %ax
	cmpw	$0x100, %ax
	ja	e801_more_than_16mb
	shlw	$6, %ax				/* multiply by 64 */
	subw	$0x400, %ax			/* 1st meg does not count */

gs	movw	%ax, OFFS_AX(%bp)		/* return memory size between 1M and 16M in 1kb chunks in AX and CX */
gs	movw	%ax, OFFS_CX(%bp)
gs	movw	$0, OFFS_BX(%bp)		/* set BX and DX to 0*/
gs	movw	$0, OFFS_DX(%bp)
gs	movw	OFFS_FLAGS(%bp), %ax
	andw	$0xfffe, %ax			/* clear carry -- function succeeded */
gs	movw	%ax, OFFS_FLAGS(%bp)
	xorw	%ax, %ax
	ret

e801_more_than_16mb:
	subw	$0x100, %ax			/* subtract 16MB */

gs	movw	$0x3c00, OFFS_AX(%bp)		/* return 0x3c00 (16MB-1MB) in AX and CX */
gs	movw	$0x3c00, OFFS_CX(%bp)
gs	movw	%ax, OFFS_BX(%bp)		/* set BX and DX to number of 64kb chunks above 16MB */
gs	movw	%ax, OFFS_DX(%bp)

gs	movw	OFFS_FLAGS(%bp), %ax
	andw	$0xfffe, %ax			/* clear carry -- function succeeded */
gs	movw	%ax, OFFS_FLAGS(%bp)
	xorw	%ax, %ax
	ret

Lfunc_88h:
cs	movw	ram_in_64kb_chunks, %ax
	cmpw	$0x100, %ax
	jna	b88_not_more_than16
	movw	$0x100, %ax
b88_not_more_than16:
	shlw	$6, %ax
	subw	$0x400, %ax			/* 1st meg does not count */

gs	movw	%ax, OFFS_AX(%bp)		/* return number of kilobytes between 16MB and 16MB in ax */

gs	movw	OFFS_FLAGS(%bp), %ax
	andw	$0xfffe, %ax			/* clear carry -- function succeeded */
gs	movw	%ax, OFFS_FLAGS(%bp)

	xorw	%ax, %ax
	ret


/*
 ************************************************************
 * BIOS interrupt 16h -- keyboard services
 ************************************************************
 */
bios_16h:
gs	movw	OFFS_AX(%bp), %ax
	shrw	$8, %ax
	cmpw	$0x03, %ax
	je	Lfunc_03h
	movw	$0xffff, %ax
	ret
Lfunc_03h:
	xorw	%ax, %ax			/* do nothing -- function not supported */
	ret

/*
 ************************************************************
 * BIOS interrupt 1ah -- PCI bios
 ************************************************************
 */
bios_1ah:
gs	movw	OFFS_AX(%bp), %ax
	cmpb	$0xb1, %ah
	je	Lfunc_b1h
	movw	$0xffff, %ax
	ret
Lfunc_b1h:
	call	realmode_pci_bios
	xorw	%ax, %ax			/* do nothing -- function not supported */
	ret


.globl ram_in_64kb_chunks
.hidden ram_in_64kb_chunks
.type ram_in_64kb_chunks, @function
ram_in_64kb_chunks:
	.word	0

.globl bios_equipment
.hidden bios_equipment
.type bios_equipment, @function
bios_equipment:
	.word	0

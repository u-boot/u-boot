/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB <daniel@omicron.se>.
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

/* now setup the General purpose bus to give us access to the LEDs.
 * We can then use the leds to display status information.
 */

sc520_cdp_registers:
/* size    offset        value */
.word 1 ; .word 0x040 ; .long 0x00       /* SDRAM buffer control */
.word 2 ; .word 0xc08 ; .long 0x0001     /* GP CS offset */
.word 2 ; .word 0xc09 ; .long 0x0003     /* GP CS width */
.word 2 ; .word 0xc0a ; .long 0x0001     /* GP CS width */
.word 2 ; .word 0xc0b ; .long 0x0003     /* GP RD pulse width */
.word 2 ; .word 0xc0c ; .long 0x0001     /* GP RD offse */
.word 2 ; .word 0xc0d ; .long 0x0003     /* GP WR pulse width */
.word 2 ; .word 0xc0e ; .long 0x0001     /* GP WR offset */
.word 2 ; .word 0xc2c ; .long 0x0000     /* GPIO directionreg */
.word 2 ; .word 0xc2a ; .long 0x0000     /* GPIO directionreg */
.word 2 ; .word 0xc22 ; .long 0xffff     /* GPIO pin function 31-16 reg */
.word 2 ; .word 0xc20 ; .long 0xffff     /* GPIO pin function 15-0 reg */
.word 2 ; .word 0x0c4 ; .long 0x28000680 /* PAR 15 for access to led 680 */
.word 0 ; .word 0x000 ; .long 0x00

/* board early intialization */
.globl early_board_init
early_board_init:
	movl    $sc520_cdp_registers,%esi
init_loop:
	movl    $0xfffef000,%edi     /* MMCR base to edi */
	movw	(%esi), %bx          /* load sizer to bx */
	cmpw	$0, %bx              /* if sie is 0 we're done */
	je	done
	xorl	%edx,%edx
	movw	2(%esi), %dx        /* load MMCR offset to dx */
	addl	%edx, %edi          /* add offset to base in edi */
	movl	4(%esi), %eax       /* load value in eax */
	cmpw	$1, %bx
	je	byte                 /* byte op? */
	cmpw	$2, %bx
	je	word                 /* word op? */
	movl	%eax, (%edi)         /* must be long, then */
	jmp	next
byte:   movb    %al,(%edi)
	jmp	next
word:	movw	%ax,(%edi)
next:	addl	$8, %esi             /* advance esi */
	jmp	init_loop

	/* the leds ad 0x80 and 0x680 should now work */
done:   movb	$0x88, %al
	out	%al, $0x80
	movw	$0x680, %dx
	out	%al, %dx

	jmp 	*%ebp		     /* return to caller */


.globl __show_boot_progress
__show_boot_progress:
	out	%al, $0x80
	xchg	%al, %ah
	movw	$0x680, %dx
	out	%al, %dx
	jmp	*%ebp

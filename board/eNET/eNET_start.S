/*
 * (C) Copyright 2008
 * Graeme Russ, graeme.russ@gmail.com.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include "hardware.h"

/* board early intialization */
.globl early_board_init
early_board_init:
	/* No 32-bit board specific initialisation */
	jmp	*%ebp		/* return to caller */

.globl show_boot_progress_asm
show_boot_progress_asm:

	movb	%al, %dl	/* Create Working Copy */
	andb	$0x80, %dl	/* Mask in only Error bit */
	shrb	$0x02, %dl	/* Shift Error bit to Error LED */
	andb	$0x0f, %al	/* Mask out 'Error' bit */
	orb	%dl, %al	/* Mask in ERR LED */
	movw	$LED_LATCH_ADDRESS, %dx
	outb	%al, %dx
	jmp	*%ebp		/* return to caller */

.globl cpu_halt_asm
cpu_halt_asm:
	movb	$0x0f, %al
	movw	$LED_LATCH_ADDRESS, %dx
	outb	%al, %dx
	hlt
	jmp cpu_halt_asm

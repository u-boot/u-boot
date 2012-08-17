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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * 16bit initialization code.
 * This code have to map the area of the boot flash
 * that is used by U-boot to its final destination.
 */

#include "config.h"
#include "hardware.h"
#include <asm/arch/sc520.h>
#include <generated/asm-offsets.h>

.text
.section .start16, "ax"
.code16
.globl board_init16
board_init16:
	/* Alias MMCR to 0xdf000 */
	movw	$0xfffc, %dx
	movl	$0x800df0cb, %eax
	outl	%eax, %dx

	/* Set ds to point to MMCR alias */
	movw	$0xdf00, %ax
	movw	%ax, %ds

	/* Map PAR for Boot Flash (BOOTCS, 512kB @ 0x380000000) */
	movl    $GENERATED_SC520_PAR14, %edi
	movl	$CONFIG_SYS_SC520_BOOTCS_PAR, %eax
	movl	%eax, (%di)

	/* Map PAR for LED, Hex Switches (GPCS6, 20 Bytes @ 0x1000) */
	movl    $GENERATED_SC520_PAR15, %edi
	movl	$CONFIG_SYS_SC520_LLIO_PAR, %eax
	movl	%eax, (%di)

	/* Disabe MMCR alias */
	movw	$0xfffc, %dx
	movl	$0x000000cb, %eax
	outl	%eax, %dx

	jmp	board_init16_ret

.section .bios, "ax"
.code16
.globl realmode_reset
.hidden realmode_reset
.type realmode_reset, @function
realmode_reset:
	/* Alias MMCR to 0xdf000 */
	movw	$0xfffc, %dx
	movl	$0x800df0cb, %eax
	outl	%eax, %dx

	/* Set ds to point to MMCR alias */
	movw	$0xdf00, %ax
	movw	%ax, %ds

	/* issue software reset thorugh MMCR */
	movl    $0xd72, %edi
	movb	$0x01, %al
	movb	%al, (%di)

1:	hlt
	jmp	1

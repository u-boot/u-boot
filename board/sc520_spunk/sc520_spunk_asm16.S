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

/*
 * 16bit initialization code.
 * This code have to map the area of the boot flash
 * that is used by U-boot to its final destination.
 */

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

	/* Map the entire flash at 0x38000000
	 * (with BOOTCS and PAR14, use 0xabfff800 for ROMCS1) */
	movl    $0xc0, %edi
	movl	$0x8bfff800, %eax
	movl	%eax, (%di)

	/* Disable SDRAM write buffer */
	movw    $0x40,%di
	xorw    %ax,%ax
	movb    %al, (%di)

	/* Disabe MMCR alias */
	movw	$0xfffc, %dx
	movl	$0x000000cb, %eax
	outl	%eax, %dx

	/* the return address is stored in bp */
	jmp	*%bp


.section .bios, "ax"
.code16
.globl realmode_reset
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

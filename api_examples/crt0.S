/*
 * (C) Copyright 2007 Semihalf
 *
 * Written by: Rafal Jaworowski <raj@semihalf.com>
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
 *
 */

#if defined(CONFIG_PPC)

	.text

	.globl _start
_start:
	b	main


	.globl syscall
syscall:
	lis	%r11, syscall_ptr@ha
	addi	%r11, %r11, syscall_ptr@l
	lwz	%r11, 0(%r11)
	mtctr	%r11
	bctr


	.globl syscall_ptr
syscall_ptr:
	.align	4
	.long	0
#else
#error No support for this arch!
#endif

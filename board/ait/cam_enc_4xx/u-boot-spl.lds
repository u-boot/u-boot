/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
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

MEMORY { .sram : ORIGIN = CONFIG_SPL_TEXT_BASE,\
		LENGTH = CONFIG_SPL_MAX_SIZE }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = CONFIG_SPL_TEXT_BASE;

	. = ALIGN(4);
	.text      :
	{
	__start = .;
	  arch/arm/cpu/arm926ejs/start.o	(.text)
	  *(.text*)
	} >.sram

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } >.sram

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sram
	. = ALIGN(4);
	.u_boot_list : {
		#include <u-boot.lst>
	} >.sram
	. = ALIGN(4);
	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	} >.sram

	.dynsym : {
		__dynsym_start = .;
		*(.dynsym)
	} >.sram

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end__ = .;
	} >.sram

	__image_copy_end = .;
	_end = .;
}

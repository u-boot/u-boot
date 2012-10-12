/*
 *  Copyright (C) 2012 Altera Corporation <www.altera.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

MEMORY { .sdram : ORIGIN = (0), LENGTH = (0xffffffff) }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text	:
	{
		arch/arm/cpu/armv7/start.o	(.text)
		*(.text*)
	} >.sdram

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) } >.sdram

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } >.sdram

	. = ALIGN(4);
	__image_copy_end = .;
	_end = .;

	.bss : {
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end__ = .;
	} >.sdram

	. = ALIGN(8);
	__malloc_start = .;
	. = . + CONFIG_SPL_MALLOC_SIZE;
	__malloc_end = .;

	. = . + CONFIG_SPL_STACK_SIZE;
	. = ALIGN(8);
	__stack_start = .;
}

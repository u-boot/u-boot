/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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


OUTPUT_FORMAT("elf32-nios")
OUTPUT_ARCH(nios)
ENTRY(_start)

SECTIONS
{
	.text :
	{
	  *(.text)
	}
	__text_end = .;

	. = ALIGN(4);
	.rodata :
	{
		*(.rodata)
	}
	__rodata_end = .;

	. = ALIGN(4);
	.data :
	{
		*(.data)
	}
	. = ALIGN(4);
	__data_end = .;

	__bss_start = .;
	. = ALIGN(4);
	.bss :
	{
		*(.bss)
	}
	. = ALIGN(4);
	__bss_end = .;
	_end = .;
}

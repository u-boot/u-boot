/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2008 Freescale Semiconductor, Inc.
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

OUTPUT_ARCH(powerpc)
SECTIONS
{
	. = 0xfff00000;
	.text : {
		*(.text*)
		. = ALIGN(16);
		*(.eh_frame)
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(8);
	.data : {
		*(.data*)
		*(.sdata*)
		_GOT2_TABLE_ = .;
		*(.got2)
		__got2_entries = (. - _GOT2_TABLE_) >> 2;
	}

	. = ALIGN(8);
	__bss_start = .;
	.bss (NOLOAD) : { *(.*bss) }
	_end = .;
}
ENTRY(_start)
ASSERT(_end <= 0xfff01000, "NAND bootstrap too big");

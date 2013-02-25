/*
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * January 2004 - Changed to support H4 device
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = CONFIG_SPL_TEXT_BASE;
	.text.0	:
	{
		arch/arm/cpu/pxa/start.o		(.text*)
		board/vpac270/libvpac270.o		(.text*)
		drivers/mtd/onenand/libonenand.o	(.text*)
	}


	/* Start of the rest of the SPL */
	. = CONFIG_SPL_TEXT_BASE + 0x800;

	.text.1	:
	{
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data)
	}

	. = ALIGN(4);

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	.dynsym : {
		__dynsym_start = .;
		*(.dynsym)
	}

	. = ALIGN(0x800);

	_end = .;

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.bss)
		 . = ALIGN(4);
		__bss_end__ = .;
	}

	/DISCARD/ : { *(.bss*) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynsym*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.hash*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}

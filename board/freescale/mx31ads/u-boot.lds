/*
 * January 2004 - Changed to support H4 device
 * Copyright (c) 2004 Texas Instruments
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
	. = 0x00000000;

	. = ALIGN(4);
	.text	   :
	{
	  /* WARNING - the following is hand-optimized to fit within	*/
	  /* the sector layout of our flash chips!	XXX FIXME XXX	*/

	  arch/arm/cpu/arm1136/start.o			(.text)
	  board/freescale/mx31ads/libmx31ads.o	(.text)
	  arch/arm/lib/libarm.o			(.text)
	  net/libnet.o				(.text)
	  drivers/mtd/libmtd.o			(.text)

	  . = DEFINED(env_offset) ? env_offset : .;
	  common/env_embedded.o(.text)

	  *(.text)
	}
	. = ALIGN(4);
	.rodata : { *(.rodata) }

	. = ALIGN(4);
	.data : {
		*(.data)
	}

	. = ALIGN(4);

	. = ALIGN(4);
	.u_boot_list : {
	#include <u-boot.lst>
	}

	. = ALIGN(4);

	__image_copy_end = .;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	.dynsym : {
		__dynsym_start = .;
		*(.dynsym)
	}

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

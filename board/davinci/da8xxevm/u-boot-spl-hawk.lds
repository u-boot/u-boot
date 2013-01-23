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

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0xc1080000;

	. = ALIGN(4);
	.text      :
	{
	  arch/arm/cpu/arm926ejs/start.o		(.text)
	  arch/arm/cpu/arm926ejs/davinci/libdavinci.o	(.text)
	  drivers/mtd/nand/libnand.o			(.text)

	  *(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(.rodata) }

	. = ALIGN(4);
	.data : {
		*(.data)
	__datarel_start = .;
		*(.data.rel)
	__datarelrolocal_start = .;
		*(.data.rel.ro.local)
	__datarellocal_start = .;
		*(.data.rel.local)
	__datarelro_start = .;
		*(.data.rel.ro)
	}

	. = ALIGN(4);
	.u_boot_list : {
		#include <u-boot.lst>
	}

	. = ALIGN(4);
	__rel_dyn_start = .;
	__rel_dyn_end = .;
	__dynsym_start = .;

	__got_start = .;
	. = ALIGN(4);
	.got : { *(.got) }

	__got_end = .;

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end__ = .;
	}

	_end = .;
}

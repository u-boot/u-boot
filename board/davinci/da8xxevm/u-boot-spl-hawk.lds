/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetki, DENX Software Engineering, <lg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
	  arch/arm/cpu/arm926ejs/start.o		(.text*)
	  arch/arm/cpu/arm926ejs/davinci/built-in.o	(.text*)
	  drivers/mtd/nand/built-in.o			(.text*)

	  *(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(.rodata*) }

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
	__image_copy_end = .;
	__rel_dyn_start = .;
	__rel_dyn_end = .;

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
		__bss_end = .;
	}

	.end :
	{
		*(.__end)
	}
}

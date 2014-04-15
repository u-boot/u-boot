/*
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <gj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text      :
	{
		*(.__image_copy_start)
		*(.vectors)
		arch/arm/cpu/arm920t/start.o	(.text*)
		/* the EP93xx expects to find the pattern 'CRUS' at 0x1000 */
	  . = 0x1000;
	  LONG(0x53555243)
	  *(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(.rodata*) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	.got : { *(.got) }

	. = .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	__bss_start = .;
	.bss : { *(.bss*) }
	__bss_end = .;

	.end :
	{
		*(.__end)
	}
}

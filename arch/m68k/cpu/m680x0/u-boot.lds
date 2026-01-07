// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Linker Script for m68040
 *
 * Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

OUTPUT_ARCH(m68k)
ENTRY(_start)

SECTIONS
{
	. = 0x00000000;
	__text_start = .;

	.text :
	{
		arch/m68k/cpu/m680x0/start.o (.text*)
		*(.text*)
	}

	. = ALIGN(16);
	.rodata : { *(.rodata*) }

	. = ALIGN(16);
	.data : { *(.data*) }

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(*u_boot_list*)));
	}

	. = ALIGN(4);
	__image_copy_end = .;
	__init_end = .;

	. = ALIGN(16);
	__bss_start = .;
	.bss :
	{
		*(.bss*)
		. = ALIGN(16);
	}
	__bss_end = .;

	_end = .;
}

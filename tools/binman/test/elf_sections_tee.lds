/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 * Copyright (c) 2025 Canonical Ltd.
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x00100010;
	_start = .;

	. = ALIGN(4);
	.text :
	{
		*(.text*)
	}

	. = 0x00101000;
	.sram :
	{
		*(.sram*)
	}

	/DISCARD/ : {
		*(.comment)
		*(.dyn*)
	}
}

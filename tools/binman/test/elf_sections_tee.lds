/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2016 Google, Inc
 * Copyright (c) 2026 Canonical Ltd.
 *
 * Mirrors elf_sections.lds with the addresses shifted up by 0x100000 so
 * the TEE image does not overlap the ATF image; keep the two in sync.
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

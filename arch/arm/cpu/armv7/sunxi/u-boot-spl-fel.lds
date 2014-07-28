/*
 * (C) Copyright 2013
 * Henrik Nordstrom <henrik@henriknordstrom.net>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(s_init)
SECTIONS
{
	. = 0x00002000;

	. = ALIGN(4);
	.text :
	{
		*(.text.s_init)
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	. = .;

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

	. = ALIGN(4);
	.note.gnu.build-id :
	{
		*(.note.gnu.build-id)
	}
	_end = .;

	. = ALIGN(4096);
	.mmutable : {
		*(.mmutable)
	}

	.bss_start __rel_dyn_start (OVERLAY) : {
		KEEP(*(.__bss_start));
		__bss_base = .;
	}

	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		. = ALIGN(4);
		__bss_limit = .;
	}

	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));
	}

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
	/DISCARD/ : { *(.note*) }
}

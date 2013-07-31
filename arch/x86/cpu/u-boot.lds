/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = CONFIG_SYS_TEXT_BASE;	/* Location of bootcode in flash */
	__text_start = .;
	.text  : { *(.text*); }

	. = ALIGN(4);

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	.hash : { *(.hash*) }

	. = ALIGN(4);
	.got : { *(.got*) }

	. = ALIGN(4);
	__data_end = .;
	__init_end = .;

	. = ALIGN(4);
	.dynsym : { *(.dynsym*) }

	. = ALIGN(4);
	__rel_dyn_start = .;
	.rel.dyn : { *(.rel.dyn) }
	__rel_dyn_end = .;
	. = ALIGN(4);
	_end = .;

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.bss)
		*(COM*)
		. = ALIGN(4);
		__bss_end = .;
	}

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }

#ifdef CONFIG_X86_RESET_VECTOR

	/*
	 * The following expressions place the 16-bit Real-Mode code and
	 * Reset Vector at the end of the Flash ROM
	 */
	. = START_16;
	.start16 : AT (CONFIG_SYS_TEXT_BASE + (CONFIG_SYS_MONITOR_LEN - RESET_SEG_SIZE + START_16)) { KEEP(*(.start16)); }

	. = RESET_VEC_LOC;
	.resetvec : AT (CONFIG_SYS_TEXT_BASE + (CONFIG_SYS_MONITOR_LEN - RESET_SEG_SIZE + RESET_VEC_LOC)) { KEEP(*(.resetvec)); }
#endif
}

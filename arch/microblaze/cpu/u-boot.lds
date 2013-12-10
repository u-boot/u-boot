/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_ARCH(microblaze)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(0x4):
	{
		__text_start = .;
		arch/microblaze/cpu/start.o (.text)
		*(.text)
		__text_end = .;
	}

	.rodata ALIGN(0x4):
	{
		__rodata_start = .;
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		__rodata_end = .;
	}

	.data ALIGN(0x4):
	{
		__data_start = .;
#ifdef CONFIG_OF_EMBED
		dts/built-in.o (.data)
#endif
		*(.data)
		__data_end = .;
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	.bss ALIGN(0x4):
	{
		__bss_start = .;
		*(.sbss)
		*(.scommon)
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	__end = . ;
}

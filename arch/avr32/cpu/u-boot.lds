/* -*- Fundamental -*-
 *
 * Copyright (C) 2005-2006 Atmel Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
OUTPUT_FORMAT("elf32-avr32", "elf32-avr32", "elf32-avr32")
OUTPUT_ARCH(avr32)
ENTRY(_start)

SECTIONS
{
	. = 0;
	_text = .;
	.text : {
		*(.exception.text)
		*(.text)
		*(.text.*)
	}
	_etext = .;

	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(8);
	_data = .;
	.data : {
		*(.data)
		*(.data.*)
	}

	. = ALIGN(4);

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	_got = .;
	.got : {
		*(.got)
	}
	_egot = .;

	. = ALIGN(8);
	_edata = .;

	.bss (NOLOAD) : {
		*(.bss)
		*(.bss.*)
	}
	. = ALIGN(8);
	__bss_end = .;
}

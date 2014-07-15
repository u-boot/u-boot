/*
 * Copyright (C) 2013-2014 Synopsys, Inc. All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT("elf32-littlearc", "elf32-littlearc", "elf32-littlearc")
OUTPUT_ARCH(arc)
ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.text :	{
		*(.__text_start)
		*(.__image_copy_start)
		CPUDIR/start.o (.text*)
		*(.text*)
	}

	. = ALIGN(4);
	.text_end :
	{
		*(.__text_end)
	}

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	.rel_dyn_start : {
		*(.__rel_dyn_start)
	}

	.rela.dyn : {
		*(.rela.dyn)
	}

	.rel_dyn_end : {
		*(.__rel_dyn_end)
	}

	. = ALIGN(4);
	.bss_start : {
		*(.__bss_start);
	}

	.bss : {
		*(.bss*)
	}

	.bss_end : {
		*(.__bss_end);
	}

	. = ALIGN(4);
	.image_copy_end : {
		*(.__image_copy_end)
		*(.__init_end)
	}
}

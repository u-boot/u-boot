/*
 * Copyright (c) 2014 Xilinx, Inc. Michal Simek
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

MEMORY { .sram : ORIGIN = CONFIG_SPL_TEXT_BASE,\
		LENGTH = CONFIG_SPL_MAX_SIZE }
MEMORY { .sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR, \
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE }

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = ALIGN(4);
	.text :
	{
		__image_copy_start = .;
		*(.vectors)
		CPUDIR/start.o (.text*)
		*(.text*)
	} > .sram

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	} > .sram

	. = ALIGN(4);
	.data : {
		*(.data*)
	} > .sram

	. = ALIGN(4);
#ifdef CONFIG_SPL_DM
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list_*_driver_*)));
		KEEP(*(SORT(.u_boot_list_*_uclass_*)));
	} > .sram

	. = ALIGN(4);
#endif

	. = .;

	_image_binary_end = .;

	_end = .;

	/* Move BSS section to RAM because of FAT */
	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss*)
		 . = ALIGN(4);
		__bss_end = .;
	} > .sdram

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}

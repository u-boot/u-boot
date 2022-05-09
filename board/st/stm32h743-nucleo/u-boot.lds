/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

#include <config.h>
#include <asm/psci.h>

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
    {
    FLASH : ORIGIN = 0x08000000, LENGTH = 0x00200000
    RAM:    ORIGIN = 0x24000000, LENGTH = 0x00080000
    RAM2:   ORIGIN = 0x30000000, LENGTH = 0x00048000
    }

ENTRY(_start)

SECTIONS
{
	.text :
	{
        _stext = .;
        __image_copy_start = .;
		*(.vectors)
		CPUDIR/start.o (.text*)
	} > FLASH

	.text_rest :
	{
		*(.text*)
	} > FLASH

	. = ALIGN(4);
	.rodata :
    {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
    }  > FLASH

	. = ALIGN(4);
	.u_boot_list :
    {
		KEEP(*(SORT(.u_boot_list*)));
        _etext = .;
        __image_copy_end = .;
	} > FLASH


	. = ALIGN(4);
    __data_in_flash = .;
	.data :
    {
        _sdata = .;
		*(.data*)
        _edata = .;
	} > RAM2 AT > FLASH


/*
 * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
 * __bss_base and __bss_limit are for linker only (overlay ordering)
 */

	. = ALIGN(4);
	.bss_start :
    {
		KEEP(*(.__bss_start));
		__bss_base = .;
	} > RAM2

	.bss :
    {
		*(.bss*)
		 . = ALIGN(4);
		 __bss_limit = .;
	} > RAM2

	.bss_end :
    {
		KEEP(*(.__bss_end));
	} > RAM2

    gd = CONFIG_SYS_GLOBAL_DATA_POINTER;


}

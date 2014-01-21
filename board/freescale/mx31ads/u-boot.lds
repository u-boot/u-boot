/*
 * January 2004 - Changed to support H4 device
 * Copyright (c) 2004 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text	   :
	{
		*(.__image_copy_start)
	  /* WARNING - the following is hand-optimized to fit within	*/
	  /* the sector layout of our flash chips!	XXX FIXME XXX	*/

	  arch/arm/cpu/arm1136/start.o		(.text*)
	  board/freescale/mx31ads/built-in.o	(.text*)
	  arch/arm/lib/built-in.o		(.text*)
	  net/built-in.o			(.text*)
	  drivers/mtd/built-in.o		(.text*)

	  . = DEFINED(env_offset) ? env_offset : .;
	  common/env_embedded.o(.text*)

	  *(.text*)
	}
	. = ALIGN(4);
	.rodata : { *(.rodata*) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rel.dyn : {
		*(.rel*)
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}

	_end = .;

/*
 * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
 * __bss_base and __bss_limit are for linker only (overlay ordering)
 */

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

	.dynsym _end : { *(.dynsym) }
	.dynbss : { *(.dynbss) }
	.dynstr : { *(.dynstr*) }
	.dynamic : { *(.dynamic*) }
	.hash : { *(.hash*) }
	.plt : { *(.plt*) }
	.interp : { *(.interp*) }
	.gnu : { *(.gnu*) }
	.ARM.exidx : { *(.ARM.exidx*) }
}

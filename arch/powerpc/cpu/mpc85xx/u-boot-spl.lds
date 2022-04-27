/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de
 *
 * Copyright 2009 Freescale Semiconductor, Inc.
 */

#include "config.h"

OUTPUT_ARCH(powerpc)
#ifdef CONFIG_SYS_MPC85XX_NO_RESETVEC
PHDRS
{
	text PT_LOAD;
	bss PT_LOAD;
}
#endif
SECTIONS
{
/* For ifc, elbc, esdhc, espi, all need the SPL without section .resetvec */
#ifdef CONFIG_SYS_MPC85XX_NO_RESETVEC
	.bootpg IMAGE_TEXT_BASE - 0x1000 :
	{
		KEEP(*(.bootpg))
	} :text = 0xffff
#endif
	. = IMAGE_TEXT_BASE;
	.text : {
		*(.text*)
	}
	_etext = .;

	.reloc : {
		_GOT2_TABLE_ = .;
		KEEP(*(.got2))
		KEEP(*(.got))
		_FIXUP_TABLE_ = .;
		KEEP(*(.fixup))
	}
	__got2_entries = ((_GLOBAL_OFFSET_TABLE_ - _GOT2_TABLE_) >> 2) - 1;
	__fixup_entries = (. - _FIXUP_TABLE_) >> 2;

	. = ALIGN(8);
	.data : {
		*(.rodata*)
		*(.data*)
		*(.sdata*)
	}
	_edata  =  .;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = .;
	__start___ex_table = .;
	__ex_table : { *(__ex_table) }
	__stop___ex_table = .;

	. = ALIGN(8);
	__init_begin = .;
	__init_end = .;
	_end = .;
#ifdef CONFIG_SPL_SKIP_RELOCATE
	. = ALIGN(4);
	__bss_start = .;
	.bss : {
		*(.sbss*)
		*(.bss*)
	}
	. = ALIGN(4);
	__bss_end = .;
#endif

/* For nor and nand is needed the SPL with section .resetvec */
#ifndef CONFIG_SYS_MPC85XX_NO_RESETVEC
#if defined(CONFIG_FSL_IFC) /* Restrict bootpg at 4K boundry for IFC */
#ifndef BOOT_PAGE_OFFSET
#define BOOT_PAGE_OFFSET 0x1000
#endif
	.bootpg IMAGE_TEXT_BASE + BOOT_PAGE_OFFSET :
	{
		arch/powerpc/cpu/mpc85xx/start.o (.bootpg)
	}
#ifndef RESET_VECTOR_OFFSET
#define RESET_VECTOR_OFFSET 0x1ffc /* IFC has 8K sram */
#endif
#elif defined(CONFIG_FSL_ELBC)
#define RESET_VECTOR_OFFSET 0xffc /* LBC has 4k sram */
#else
#error unknown NAND controller
#endif
	.resetvec IMAGE_TEXT_BASE + RESET_VECTOR_OFFSET : {
		KEEP(*(.resetvec))
	} = 0xffff
#endif

#ifndef CONFIG_SPL_SKIP_RELOCATE
	/*
	 * Make sure that the bss segment isn't linked at 0x0, otherwise its
	 * address won't be updated during relocation fixups.
	 */
	. |= 0x10;

	. = ALIGN(4);
	__bss_start = .;
	.bss : {
		*(.sbss*)
		*(.bss*)
	}
	. = ALIGN(4);
	__bss_end = .;
#endif
}

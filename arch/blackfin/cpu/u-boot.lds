/*
 * U-boot - u-boot.lds.S
 *
 * Copyright (c) 2005-2010 Analog Device Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/blackfin.h>
#undef ALIGN
#undef ENTRY

#ifndef LDS_BOARD_TEXT
# define LDS_BOARD_TEXT
#endif

/* If we don't actually load anything into L1 data, this will avoid
 * a syntax error.  If we do actually load something into L1 data,
 * we'll get a linker memory load error (which is what we'd want).
 * This is here in the first place so we can quickly test building
 * for different CPU's which may lack non-cache L1 data.
 */
#ifndef L1_DATA_A_SRAM
# define L1_DATA_A_SRAM      0
# define L1_DATA_A_SRAM_SIZE 0
#endif
#ifndef L1_DATA_B_SRAM
# define L1_DATA_B_SRAM      L1_DATA_A_SRAM
# define L1_DATA_B_SRAM_SIZE L1_DATA_A_SRAM_SIZE
#endif

/* The 0xC offset is so we don't clobber the tiny LDR jump block. */
#ifdef CONFIG_BFIN_BOOTROM_USES_EVT1
# define L1_CODE_ORIGIN L1_INST_SRAM
#else
# define L1_CODE_ORIGIN L1_INST_SRAM + 0xC
#endif

OUTPUT_ARCH(bfin)

MEMORY
{
#if CONFIG_MEM_SIZE
	ram     : ORIGIN = CONFIG_SYS_MONITOR_BASE, LENGTH = CONFIG_SYS_MONITOR_LEN
# define ram_code ram
# define ram_data ram
#else
# define ram_code l1_code
# define ram_data l1_data
#endif
	l1_code : ORIGIN = L1_CODE_ORIGIN,          LENGTH = L1_INST_SRAM_SIZE
	l1_data : ORIGIN = L1_DATA_B_SRAM,          LENGTH = L1_DATA_B_SRAM_SIZE
}

ENTRY(_start)
SECTIONS
{
	.text.pre :
	{
		arch/blackfin/cpu/start.o (.text .text.*)

		LDS_BOARD_TEXT
	} >ram_code

	.text.init :
	{
		arch/blackfin/cpu/initcode.o (.text .text.*)
	} >ram_code
	__initcode_lma = LOADADDR(.text.init);
	__initcode_len = SIZEOF(.text.init);

	.text :
	{
		*(.text .text.*)
	} >ram_code

	.rodata :
	{
		. = ALIGN(4);
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		. = ALIGN(4);
	} >ram_data

	.data :
	{
		. = ALIGN(4);
		*(.data .data.*)
		*(.data1)
		*(.sdata)
		*(.sdata2)
		*(.dynamic)
		CONSTRUCTORS
	} >ram_data


	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} >ram_data

	.text_l1 :
	{
		. = ALIGN(4);
		__stext_l1 = .;
		*(.l1.text)
		. = ALIGN(4);
		__etext_l1 = .;
	} >l1_code AT>ram_code
	__text_l1_lma = LOADADDR(.text_l1);
	__text_l1_len = SIZEOF(.text_l1);
	ASSERT (__text_l1_len <= L1_INST_SRAM_SIZE, "L1 text overflow!")

	.data_l1 :
	{
		. = ALIGN(4);
		__sdata_l1 = .;
		*(.l1.data)
		*(.l1.bss)
		. = ALIGN(4);
		__edata_l1 = .;
	} >l1_data AT>ram_data
	__data_l1_lma = LOADADDR(.data_l1);
	__data_l1_len = SIZEOF(.data_l1);
	ASSERT (__data_l1_len <= L1_DATA_B_SRAM_SIZE, "L1 data overflow!")

	.bss :
	{
		. = ALIGN(4);
		*(.sbss) *(.scommon)
		*(.dynbss)
		*(.bss .bss.*)
		*(COMMON)
		. = ALIGN(4);
	} >ram_data
	__bss_end = .;
	__bss_start = ADDR(.bss);
	__bss_len = SIZEOF(.bss);
	__init_end = .;
}

/*
 * U-boot - u-boot.lds.S
 *
 * Copyright (c) 2005-2008 Analog Device Inc.
 *
 * (C) Copyright 2000-2004
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <asm/blackfin.h>
#undef ALIGN
#undef ENTRY
#undef bfin

#ifndef LDS_BOARD_TEXT
# define LDS_BOARD_TEXT
#endif

/* If we don't actually load anything into L1 data, this will avoid
 * a syntax error.  If we do actually load something into L1 data,
 * we'll get a linker memory load error (which is what we'd want).
 * This is here in the first place so we can quickly test building
 * for different CPU's which may lack non-cache L1 data.
 */
#ifndef L1_DATA_B_SRAM
# define L1_DATA_B_SRAM      CONFIG_SYS_MONITOR_BASE
# define L1_DATA_B_SRAM_SIZE 0
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
	ram     : ORIGIN = CONFIG_SYS_MONITOR_BASE, LENGTH = CONFIG_SYS_MONITOR_LEN
	l1_code : ORIGIN = L1_CODE_ORIGIN,          LENGTH = L1_INST_SRAM_SIZE
	l1_data : ORIGIN = L1_DATA_B_SRAM,          LENGTH = L1_DATA_B_SRAM_SIZE
}

ENTRY(_start)
SECTIONS
{
	.text :
	{
		cpu/blackfin/start.o (.text .text.*)

		LDS_BOARD_TEXT

		__initcode_start = .;
		cpu/blackfin/initcode.o (.text .text.*)
		__initcode_end = .;

		*(.text .text.*)
	} >ram

	.rodata :
	{
		. = ALIGN(4);
		*(.rodata .rodata.*)
		*(.rodata1)
		*(.eh_frame)
		. = ALIGN(4);
	} >ram

	.data :
	{
		. = ALIGN(256);
		*(.data .data.*)
		*(.data1)
		*(.sdata)
		*(.sdata2)
		*(.dynamic)
		CONSTRUCTORS
	} >ram

	.u_boot_cmd :
	{
		___u_boot_cmd_start = .;
		*(.u_boot_cmd)
		___u_boot_cmd_end = .;
	} >ram

	.text_l1 :
	{
		. = ALIGN(4);
		__stext_l1 = .;
		*(.l1.text)
		. = ALIGN(4);
		__etext_l1 = .;
	} >l1_code AT>ram
	__stext_l1_lma = LOADADDR(.text_l1);

	.data_l1 :
	{
		. = ALIGN(4);
		__sdata_l1 = .;
		*(.l1.data)
		*(.l1.bss)
		. = ALIGN(4);
		__edata_l1 = .;
	} >l1_data AT>ram
	__sdata_l1_lma = LOADADDR(.data_l1);

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.sbss) *(.scommon)
		*(.dynbss)
		*(.bss .bss.*)
		*(COMMON)
		__bss_end = .;
	} >ram
}

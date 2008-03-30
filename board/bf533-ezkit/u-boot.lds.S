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

/* If we don't actually load anything into L1 data, this will avoid
 * a syntax error.  If we do actually load something into L1 data,
 * we'll get a linker memory load error (which is what we'd want).
 * This is here in the first place so we can quickly test building
 * for different CPU's which may lack non-cache L1 data.
 */
#ifndef L1_DATA_B_SRAM
# define L1_DATA_B_SRAM      CFG_MONITOR_BASE
# define L1_DATA_B_SRAM_SIZE 0
#endif

OUTPUT_ARCH(bfin)

/* The 0xC offset is so we don't clobber the tiny LDR jump block. */
MEMORY
{
	ram     : ORIGIN = CFG_MONITOR_BASE, LENGTH = CFG_MONITOR_LEN
	l1_code : ORIGIN = L1_INST_SRAM+0xC, LENGTH = L1_INST_SRAM_SIZE
	l1_data : ORIGIN = L1_DATA_B_SRAM,   LENGTH = L1_DATA_B_SRAM_SIZE
}

SECTIONS
{
	.text :
	{
#ifdef ENV_IS_EMBEDDED
		/* WARNING - the following is hand-optimized to fit within
		 * the sector before the environment sector. If it throws
		 * an error during compilation remove an object here to get
		 * it linked after the configuration sector.
		 */

		cpu/blackfin/start.o		(.text)
		cpu/blackfin/traps.o		(.text)
		cpu/blackfin/interrupt.o	(.text)
		cpu/blackfin/serial.o		(.text)
		common/dlmalloc.o		(.text)
		lib_generic/crc32.o		(.text)
		lib_generic/zlib.o		(.text)
		board/bf533-ezkit/bf533-ezkit.o		(.text)

		. = DEFINED(env_offset) ? env_offset : .;
		common/environment.o	(.text)
#endif

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

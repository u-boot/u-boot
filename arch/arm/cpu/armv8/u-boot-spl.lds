/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *	Aneesh V <aneesh@ti.com>
 */

MEMORY { .sram : ORIGIN = IMAGE_TEXT_BASE,
		LENGTH = IMAGE_MAX_SIZE }
#ifdef CONFIG_SPL_SEPARATE_BSS
MEMORY { .sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR,
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE }
#endif

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
	__image_copy_start = ADDR(.text);
	.text : {
		. = ALIGN(8);
		CPUDIR/start.o (.text*)
		*(.text*)
	} >.sram

	.rodata : {
		. = ALIGN(8);
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	} >.sram

	.data : {
		. = ALIGN(8);
		*(.__data_start)
		*(.data*)
	} >.sram

#ifdef CONFIG_SPL_RECOVER_DATA_SECTION
	.data_save : {
		*(.__data_save_start)
		. = SIZEOF(.data);
		*(.__data_save_end)
	} >.sram
#endif

	__u_boot_list : {
		. = ALIGN(8);
		KEEP(*(SORT(__u_boot_list*)));
	} >.sram

	. = ALIGN(8);
	__image_copy_end = .;
	_end = .;
	_image_binary_end = .;

#ifdef CONFIG_SPL_SEPARATE_BSS
	.bss : {
		__bss_start = .;
		*(.bss*)
		. = ALIGN(8);
		__bss_end = .;
	} >.sdram
#else
	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss*)
		 . = ALIGN(8);
		__bss_end = .;
	} >.sram
#endif
	__bss_size = __bss_end - __bss_start;

	/DISCARD/ : { *(.rela*) }
	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }

#ifdef CONFIG_LINUX_KERNEL_IMAGE_HEADER
#include "linux-kernel-image-header-vars.h"
#endif
}

ASSERT(ADDR(.bss) % 8 == 0, \
       ".bss must be 8-byte aligned");

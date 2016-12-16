/*
 * (C) Copyright 2013
 * David Feng <fenghua@phytium.com.cn>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <config.h>
#include <asm/psci.h>

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
#ifdef CONFIG_ARMV8_SECURE_BASE
	/DISCARD/ : { *(.rela._secure*) }
#endif
	. = 0x00000000;

	. = ALIGN(8);
	.text :
	{
		*(.__image_copy_start)
		CPUDIR/start.o (.text*)
		*(.text*)
	}

#ifdef CONFIG_ARMV8_PSCI
	.__secure_start :
#ifndef CONFIG_ARMV8_SECURE_BASE
		ALIGN(CONSTANT(COMMONPAGESIZE))
#endif
	{
		KEEP(*(.__secure_start))
	}

#ifndef CONFIG_ARMV8_SECURE_BASE
#define CONFIG_ARMV8_SECURE_BASE
#define __ARMV8_PSCI_STACK_IN_RAM
#endif
	.secure_text CONFIG_ARMV8_SECURE_BASE :
		AT(ADDR(.__secure_start) + SIZEOF(.__secure_start))
	{
		*(._secure.text)
	}

	.secure_data : AT(LOADADDR(.secure_text) + SIZEOF(.secure_text))
	{
		*(._secure.data)
	}

	.secure_stack ALIGN(ADDR(.secure_data) + SIZEOF(.secure_data),
			    CONSTANT(COMMONPAGESIZE)) (NOLOAD) :
#ifdef __ARMV8_PSCI_STACK_IN_RAM
		AT(ADDR(.secure_stack))
#else
		AT(LOADADDR(.secure_data) + SIZEOF(.secure_data))
#endif
	{
		KEEP(*(.__secure_stack_start))

		. = . + CONFIG_ARMV8_PSCI_NR_CPUS * ARM_PSCI_STACK_SIZE;

		. = ALIGN(CONSTANT(COMMONPAGESIZE));

		KEEP(*(.__secure_stack_end))
	}

#ifndef __ARMV8_PSCI_STACK_IN_RAM
	. = LOADADDR(.secure_stack);
#endif

	.__secure_end : AT(ADDR(.__secure_end)) {
		KEEP(*(.__secure_end))
		LONG(0x1d1071c);	/* Must output something to reset LMA */
	}
#endif

	. = ALIGN(8);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(8);
	.data : {
		*(.data*)
	}

	. = ALIGN(8);

	. = .;

	. = ALIGN(8);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(8);

	.efi_runtime : {
                __efi_runtime_start = .;
		*(efi_runtime_text)
		*(efi_runtime_data)
                __efi_runtime_stop = .;
	}

	.efi_runtime_rel : {
                __efi_runtime_rel_start = .;
		*(.relaefi_runtime_text)
		*(.relaefi_runtime_data)
                __efi_runtime_rel_stop = .;
	}

	. = ALIGN(8);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	. = ALIGN(8);

	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rela.dyn : {
		*(.rela*)
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}

	_end = .;

	. = ALIGN(8);

	.bss_start : {
		KEEP(*(.__bss_start));
	}

	.bss : {
		*(.bss*)
		 . = ALIGN(8);
	}

	.bss_end : {
		KEEP(*(.__bss_end));
	}

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}

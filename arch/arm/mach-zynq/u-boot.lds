/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2004-2008 Texas Instruments
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	__image_copy_start = ADDR(.text);
	.text :
	{
		*(.vectors)
		CPUDIR/start.o (.text*)
	}

	/* This needs to come before *(.text*) */
	.efi_runtime : {
		__efi_runtime_start = .;
		*(.text.efi_runtime*)
		*(.rodata.efi_runtime*)
		*(.data.efi_runtime*)
		__efi_runtime_stop = .;
	}

	.text_rest :
	{
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = ALIGN(4);

	. = .;

	. = ALIGN(4);
	__u_boot_list : {
		KEEP(*(SORT(__u_boot_list*)));
	}

	.efi_runtime_rel : {
		__efi_runtime_rel_start = .;
		*(.rel*.efi_runtime)
		*(.rel*.efi_runtime.*)
		__efi_runtime_rel_stop = .;
	}

	. = ALIGN(8);
	__image_copy_end = .;

	.rel.dyn ALIGN(8) : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	_end = .;
	_image_binary_end = .;

/*
 * These sections occupy the same memory, but their lifetimes do
 * not overlap: U-Boot initializes .bss only after applying dynamic
 * relocations and therefore after it doesn't need .rel.dyn any more.
 */
	.bss ADDR(.rel.dyn) (OVERLAY): {
		__bss_start = .;
		*(.bss*)
		. = ALIGN(8);
		__bss_end = .;
	}

	/*
	 * Zynq needs to discard these sections because the user
	 * is expected to pass this image on to tools for boot.bin
	 * generation that require them to be dropped.
	 */
	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynbss*) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
	/DISCARD/ : { *(.ARM.exidx*) }
	/DISCARD/ : { *(.gnu.linkonce.armexidx.*) }
}

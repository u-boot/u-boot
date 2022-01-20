/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * Graeme Russ, <graeme.russ@gmail.com>
 */

#ifndef _RELOCATE_H_
#define _RELOCATE_H_

#ifndef USE_HOSTCC
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;
#endif

/**
 * copy_uboot_to_ram() - Copy U-Boot to its new relocated position
 *
 * Return: 0 if OK, -ve on error
 */
int copy_uboot_to_ram(void);

/**
 * clear_bss() - Clear the BSS (Blocked Start by Symbol) segment
 *
 * This clears the memory used by global variables
 *
 * Return: 0 if OK, -ve on error
 */
int clear_bss(void);

/**
 * do_elf_reloc_fixups() - Fix up ELF relocations in the relocated code
 *
 * This processes the relocation tables to ensure that the code can run in its
 * new location.
 *
 * Return: 0 if OK, -ve on error
 */
int do_elf_reloc_fixups(void);

/**
 * manual_reloc() - Manually relocate a pointer if needed
 *
 * This is a nop in almost all cases, except for the systems with a broken gcc
 * which need to manually relocate some things.
 *
 * @ptr: Pointer to relocate
 * Return: new pointer value
 */
static inline void *manual_reloc(void *ptr)
{
#ifndef USE_HOSTCC
	if (IS_ENABLED(CONFIG_NEEDS_MANUAL_RELOC))
		return ptr + gd->reloc_off;
#endif
		return ptr;
}

#if !defined(USE_HOSTCC) && defined(CONFIG_NEEDS_MANUAL_RELOC)
#define MANUAL_RELOC(ptr)	(ptr) = manual_reloc(ptr)
#else
#define MANUAL_RELOC(ptr)	(void)(ptr)
#endif

#endif	/* _RELOCATE_H_ */

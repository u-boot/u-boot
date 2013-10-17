/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_PROCESSOR_H_
#define __ASM_PROCESSOR_H_ 1

#define X86_GDT_ENTRY_SIZE	8

#ifndef __ASSEMBLY__

enum {
	X86_GDT_ENTRY_NULL = 0,
	X86_GDT_ENTRY_UNUSED,
	X86_GDT_ENTRY_32BIT_CS,
	X86_GDT_ENTRY_32BIT_DS,
	X86_GDT_ENTRY_32BIT_FS,
	X86_GDT_ENTRY_16BIT_CS,
	X86_GDT_ENTRY_16BIT_DS,
	X86_GDT_NUM_ENTRIES
};
#else
/* NOTE: If the above enum is modified, this define must be checked */
#define X86_GDT_ENTRY_32BIT_DS	3
#define X86_GDT_NUM_ENTRIES	7
#endif

#define X86_GDT_SIZE		(X86_GDT_NUM_ENTRIES * X86_GDT_ENTRY_SIZE)

#endif

/* Assembler macros for SPARC
 *
 * (C) Copyright 2007, taken from linux asm-sparc/asmmacro.h
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __SPARC_ASMMACRO_H__
#define __SPARC_ASMMACRO_H__

#include <config.h>

/* All trap entry points _must_ begin with this macro or else you
 * lose.  It makes sure the kernel has a proper window so that
 * c-code can be called.
 */
#define SAVE_ALL_HEAD \
	sethi	%hi(trap_setup+(CONFIG_SYS_RELOC_MONITOR_BASE-CONFIG_SYS_TEXT_BASE)), %l4; \
	jmpl	%l4 + %lo(trap_setup+(CONFIG_SYS_RELOC_MONITOR_BASE-CONFIG_SYS_TEXT_BASE)), %l6;
#define SAVE_ALL \
	SAVE_ALL_HEAD \
	nop;

/* All traps low-level code here must end with this macro. */
#define RESTORE_ALL b ret_trap_entry; clr %l6;

#endif

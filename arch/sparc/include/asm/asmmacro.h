/* Assembler macros for SPARC
 *
 * (C) Copyright 2007, taken from linux asm-sparc/asmmacro.h
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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
 *
 */

#ifndef __SPARC_ASMMACRO_H__
#define __SPARC_ASMMACRO_H__

#include <config.h>

/* All trap entry points _must_ begin with this macro or else you
 * lose.  It makes sure the kernel has a proper window so that
 * c-code can be called.
 */
#define SAVE_ALL_HEAD \
	sethi	%hi(trap_setup+(CONFIG_SYS_RELOC_MONITOR_BASE-TEXT_BASE)), %l4; \
	jmpl	%l4 + %lo(trap_setup+(CONFIG_SYS_RELOC_MONITOR_BASE-TEXT_BASE)), %l6;
#define SAVE_ALL \
	SAVE_ALL_HEAD \
	nop;

/* All traps low-level code here must end with this macro. */
#define RESTORE_ALL b ret_trap_entry; clr %l6;

#endif

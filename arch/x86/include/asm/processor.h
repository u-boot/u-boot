/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se
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

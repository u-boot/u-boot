/*
 * (C) Copyright 2003
 * Wolfgang Denk Engineering, <wd@denx.de>
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

#if defined(CONFIG_64BIT)
#define PTR_COUNT_SHIFT	3
#else
#define PTR_COUNT_SHIFT	2
#endif

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text : {
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}

	. = ALIGN(4);
	.data : {
		*(.data*)
	}

	. = .;
	_gp = ALIGN(16) + 0x7ff0;

	.got : {
		*(.got)
	}

	num_got_entries = SIZEOF(.got) >> PTR_COUNT_SHIFT;

	. = ALIGN(4);
	.sdata : {
		*(.sdata*)
	}

	. = ALIGN(4);
	.u_boot_list : {
		#include <u-boot.lst>
	}

	. = ALIGN(4);
	__image_copy_end = .;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel.dyn)
		__rel_dyn_end = .;
	}

	.deadcode : {
		/*
		 * Workaround for a binutils feature (or bug?).
		 *
		 * The GNU ld from binutils puts the dynamic relocation
		 * entries into the .rel.dyn section. Sometimes it
		 * allocates more dynamic relocation entries than it needs
		 * and the unused slots are set to R_MIPS_NONE entries.
		 *
		 * However the size of the .rel.dyn section in the ELF
		 * section header does not cover the unused entries, so
		 * objcopy removes those during stripping.
		 *
		 * Create a small section here to avoid that.
		 */
		LONG(0xffffffff);
	}

	.dynsym : {
		*(.dynsym)
	}

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.sbss.*)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}

	/DISCARD/ : {
		*(.dynbss)
		*(.dynstr)
		*(.dynamic)
		*(.interp)
		*(.hash)
		*(.gnu.*)
		*(.plt)
		*(.got.plt)
		*(.rel.plt)
	}
}

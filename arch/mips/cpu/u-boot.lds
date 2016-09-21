/*
 * (C) Copyright 2003
 * Wolfgang Denk Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
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
		__text_start = .;
		*(.text*)
		__text_end = .;
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
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN(4);
	__image_copy_end = .;
	__init_end = .;

	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel.dyn)
		__rel_dyn_end = .;
	}

	.padding : {
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
		LONG(0xFFFFFFFF)
	}

	_end = .;

	.bss __rel_dyn_start (OVERLAY) : {
		__bss_start = .;
		*(.sbss.*)
		*(.bss.*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}

	.dynsym _end : {
		*(.dynsym)
	}

	.dynbss : {
		*(.dynbss)
	}

	.dynstr : {
		*(.dynstr)
	}

	.dynamic : {
		*(.dynamic)
	}

	.plt : {
		*(.plt)
	}

	.interp : {
		*(.interp)
	}

	.gnu : {
		*(.gnu*)
	}

	.MIPS.stubs : {
		*(.MIPS.stubs)
	}

	.hash : {
		*(.hash)
	}
}

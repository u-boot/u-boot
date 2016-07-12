/*
 * SPDX-License-Identifier:	GPL-2.0+
 */

MEMORY { .spl_mem : ORIGIN = CONFIG_SPL_TEXT_BASE, \
		LENGTH = CONFIG_SPL_MAX_SIZE }
MEMORY { .bss_mem : ORIGIN = CONFIG_SPL_BSS_START_ADDR, \
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE }

OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN(4);
	.text : {
		*(.text*)
	} > .spl_mem

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	} > .spl_mem

	. = ALIGN(4);
	.data : {
		*(SORT_BY_ALIGNMENT(.data*))
		*(SORT_BY_ALIGNMENT(.sdata*))
	} > .spl_mem

#ifdef CONFIG_SPL_DM
	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	} > .spl_mem
#endif

	. = ALIGN(4);
	__image_copy_end = .;

	.bss (NOLOAD) : {
		__bss_start = .;
		*(.bss*)
		*(.sbss*)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	} > .bss_mem

	.rel.dyn (NOLOAD) : {
		*(.rel.dyn)
	}

	.dynsym : {
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

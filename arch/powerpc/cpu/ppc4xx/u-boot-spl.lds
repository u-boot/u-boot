/*
 * Copyright 2012-2013 Stefan Roese <sr@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

MEMORY
{
	sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR,
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE
	flash : ORIGIN = CONFIG_SPL_TEXT_BASE,
		LENGTH = CONFIG_SYS_SPL_MAX_LEN
}

OUTPUT_ARCH(powerpc)
ENTRY(_start)
SECTIONS
{
#ifdef CONFIG_440
	.bootpg 0xfffff000 :
	{
		arch/powerpc/cpu/ppc4xx/start.o	(.bootpg)

		/*
		 * PPC440 board need a board specific object with the
		 * TLB definitions. This needs to get included right after
		 * start.o, since the first shadow TLB only covers 4k
		 * of address space.
		 */
		CONFIG_BOARDDIR/init.o	(.bootpg)
	} > flash
#endif

	.resetvec 0xFFFFFFFC :
	{
		KEEP(*(.resetvec))
	} > flash

	.text :
	{
		__start = .;
		arch/powerpc/cpu/ppc4xx/start.o	(.text)
		CONFIG_BOARDDIR/init.o	(.text)
		*(.text*)
	} > flash

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } > flash

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } > flash

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} > sdram
}

/*
 * Copyright 2012 Stefan Roese <sr@denx.de>
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
	.text :
	{
		__start = .;
		arch/powerpc/cpu/mpc5xxx/start.o	(.text)
		*(.text*)
	} > flash

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } > flash

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } > flash

	. = ALIGN(4);
	.end_align : { *(.end_align*) } > flash
	__spl_flash_end = .;

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} > sdram
}

/*
 * (C) Copyright 2003
 * Wolfgang Denk Engineering, <wd@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-bigmips")
*/
OUTPUT_FORMAT("elf64-tradbigmips", "elf64-tradbigmips", "elf64-tradlittlemips")
OUTPUT_ARCH(mips)
SECTIONS
{
	.text       :
	{
	  *(.text*)
	}

	. = ALIGN(4);
	.rodata  : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data  : { *(.data*) }

	. = .;
	_gp = ALIGN(16) + 0x7ff0;

	.got : {
	  __got_start = .;
	  *(.got)
	  __got_end = .;
	}

	.sdata  : { *(.sdata*) }

	. = ALIGN(4);
	__bss_start = .;
	.sbss (NOLOAD) : { *(.sbss*) }
	.bss (NOLOAD)  : { *(.bss*) . = ALIGN(4); }

	_end = .;
}

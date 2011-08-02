/*
 * linker script for simple init.elf
 *
 * Copyright (c) 2005-2011 Analog Device Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <config.h>
#include <asm/blackfin.h>
#undef ALIGN
#undef ENTRY

OUTPUT_ARCH(bfin)

MEMORY
{
	l1_code : ORIGIN = L1_INST_SRAM, LENGTH = L1_INST_SRAM_SIZE
}

ENTRY(_start)
SECTIONS
{
	.text.l1 : { *(.text .text.*) } >l1_code
}

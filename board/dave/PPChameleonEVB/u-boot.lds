/*
 * Copyright 2007-2009 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "config.h"	/* CONFIG_BOARDDIR */

#ifndef RESET_VECTOR_ADDRESS
#define RESET_VECTOR_ADDRESS	0xfffffffc
#endif

OUTPUT_ARCH(powerpc)

PHDRS
{
  text PT_LOAD;
  bss PT_LOAD;
}

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = + SIZEOF_HEADERS;
  .text      :
  {
    *(.text*)
   } :text
    _etext = .;
    PROVIDE (etext = .);
    .rodata    :
   {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  } :text

  /* Read-write section, merged into data segment: */
  . = (. + 0x00FF) & 0xFFFFFF00;
  _erotext = .;
  PROVIDE (erotext = .);
  .reloc   :
  {
    _GOT2_TABLE_ = .;
    KEEP(*(.got2))
    KEEP(*(.got))
    PROVIDE(_GLOBAL_OFFSET_TABLE_ = . + 4);
    _FIXUP_TABLE_ = .;
    KEEP(*(.fixup))
  }
  __got2_entries = (_FIXUP_TABLE_ - _GOT2_TABLE_) >> 2;
  __fixup_entries = (. - _FIXUP_TABLE_) >> 2;

  .data    :
  {
    *(.data*)
    *(.sdata*)
  }
  _edata  =  .;
  PROVIDE (edata = .);

  . = .;

  . = ALIGN(4);
  .u_boot_list : {
	KEEP(*(SORT(.u_boot_list*)));
  }

  . = .;
  __start___ex_table = .;
  __ex_table : { *(__ex_table) }
  __stop___ex_table = .;

  . = ALIGN(256);
  __init_begin = .;
  .text.init : { *(.text.init) }
  .data.init : { *(.data.init) }
  . = ALIGN(256);
  __init_end = .;

  ppcenv_assert = ASSERT(. < 0xFFFF8000, ".bss section too big, overlaps .ppcenv section. Please update your confguration: CONFIG_SYS_MONITOR_BASE, CONFIG_SYS_MONITOR_LEN and CONFIG_SYS_TEXT_BASE may need to be modified.");
  . = 0xFFFF8000;
  .ppcenv :
  {
    common/env_embedded.o(.ppcenv);
  }

  .resetvec RESET_VECTOR_ADDRESS :
  {
    KEEP(*(.resetvec))
  } :text = 0xffff

  . = RESET_VECTOR_ADDRESS + 0x4;

  /*
   * Make sure that the bss segment isn't linked at 0x0, otherwise its
   * address won't be updated during relocation fixups.  Note that
   * this is a temporary fix.  Code to dynamically the fixup the bss
   * location will be added in the future.  When the bss relocation
   * fixup code is present this workaround should be removed.
   */
#if (RESET_VECTOR_ADDRESS == 0xfffffffc)
  . |= 0x10;
#endif

  __bss_start = .;
  .bss (NOLOAD)       :
  {
   *(.bss*)
   *(.sbss*)
   *(COMMON)
  } :bss

  . = ALIGN(4);
  __bss_end = . ;
  PROVIDE (end = .);
}

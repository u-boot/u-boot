/*
**=====================================================================
**
** Copyright (C) 2000, 2001, 2002, 2003
** The LEOX team <team@leox.org>, http://www.leox.org
**
** LEOX.org is about the development of free hardware and software resources
**   for system on chip.
**
** Description: U-Boot port on the LEOX's ELPT860 CPU board
** ~~~~~~~~~~~
**
**=====================================================================
**
 * SPDX-License-Identifier:	GPL-2.0+
**
**=====================================================================
*/

OUTPUT_ARCH(powerpc)

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = + SIZEOF_HEADERS;
  .text      :
  {
    /* WARNING - the following is hand-optimized to fit within	*/
    /* the sector layout of our flash chips!	XXX FIXME XXX	*/

    arch/powerpc/cpu/mpc8xx/start.o	(.text*)
    arch/powerpc/cpu/mpc8xx/traps.o	(.text*)
    common/libcommon.o			(.text*)
    arch/powerpc/cpu/mpc8xx/libmpc8xx.o	(.text*)
    board/LEOX/elpt860/libelpt860.o	(.text*)
    arch/powerpc/lib/libpowerpc.o	(.text*)
/*    drivers/rtc/librtc.o		(.text*)	*/

    . = env_offset;
    common/env_embedded.o		(.text*)

    *(.text*)
  }
  _etext = .;
  PROVIDE (etext = .);
  .rodata    :
  {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  }

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
  __got2_entries = ((_GLOBAL_OFFSET_TABLE_ - _GOT2_TABLE_) >> 2) - 1;
  __fixup_entries = (. - _FIXUP_TABLE_)>>2;

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

  __bss_start = .;
  .bss (NOLOAD)       :
  {
   *(.bss*)
   *(.sbss*)
   *(COMMON)
   . = ALIGN(4);
  }
  __bss_end = . ;
  PROVIDE (end = .);
}

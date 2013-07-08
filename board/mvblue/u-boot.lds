/*
 * (C) Copyright 2001-2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_ARCH(powerpc)
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  .text      :
  {
    arch/powerpc/cpu/mpc824x/start.o		(.text*)
    lib/libgeneric.o				(.text*)
    net/libnet.o				(.text*)
    drivers/pci/libpci.o			(.text*)
    arch/powerpc/cpu/mpc824x/libmpc824x.o	(.text*)
    board/mvblue/libmvblue.o			(.text*)
    arch/powerpc/lib/libpowerpc.o		(.text*)

    . = DEFINED(env_offset) ? env_offset : .;
    common/env_embedded.o	(.ppcenv*)

    *(.text*)
    . = ALIGN(16);
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  }

  /* Read-write section, merged into data segment: */
  . = (. + 0x0FFF) & 0xFFFFF000;
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

  . = ALIGN(4096);
  __init_begin = .;
  .text.init : { *(.text.init) }
  .data.init : { *(.data.init) }
  . = ALIGN(4096);
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

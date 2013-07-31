/*
 * (C) Copyright 2007
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_ARCH(powerpc:common)
SECTIONS
{
  .resetvec 0x00800FFC :
  {
    KEEP(*(.resetvec))
  } = 0xffff

  .text      :
  {
    start.o	(.text)
    nand_boot.o	(.text)
    ndfc.o	(.text)

    *(.text)
    *(.fixup)
  }
  _etext = .;

  .data    :
  {
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
    *(.data*)
    *(.sdata*)
    __got2_start = .;
    *(.got2)
    __got2_end = .;
  }

  _edata  =  .;

  __bss_start = .;
  .bss (NOLOAD)       :
  {
   *(.sbss)
   *(.bss)
   . = ALIGN(4);
  }

  __bss_end = . ;
}

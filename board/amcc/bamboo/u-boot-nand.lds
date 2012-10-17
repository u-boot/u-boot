/*
 * (C) Copyright 2007
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
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

    arch/powerpc/cpu/ppc4xx/start.o	(.text*)
    board/amcc/bamboo/init.o		(.text*)

    /* Align to next NAND block */
    . = ALIGN(0x4000);
    common/env_embedded.o  (.ppcenv)
    /* Keep some space here for redundant env and potential bad env blocks */
    . = ALIGN(0x10000);

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
    KEEP(*(.got))
    _GOT2_TABLE_ = .;
    KEEP(*(.got2))
    _FIXUP_TABLE_ = .;
    KEEP(*(.fixup))
  }
  __got2_entries = (_FIXUP_TABLE_ - _GOT2_TABLE_) >>2;
  __fixup_entries = (. - _FIXUP_TABLE_)>>2;

  .data    :
  {
    *(.data*)
    *(.sdata*)
  }
  _edata  =  .;
  PROVIDE (edata = .);

  . = .;

  .u_boot_list : {
	#include <u-boot.lst>
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

  __bss_end__ = . ;
  PROVIDE (end = .);
}

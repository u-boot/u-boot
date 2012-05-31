/*
 * (C) Copyright 2008
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
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

OUTPUT_ARCH(powerpc:common)
SECTIONS
{
  .resetvec 0xE3003FFC :
  {
    KEEP(*(.resetvec))
  } = 0xffff

  .text      :
  {
    start.o	(.text)
    init.o	(.text)
    nand_boot.o	(.text)
    ddr2_fixed.o (.text)
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

  __bss_end__ = . ;
}

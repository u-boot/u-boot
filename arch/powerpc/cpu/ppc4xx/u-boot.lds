/*
 * Copyright 2007-2009 Freescale Semiconductor, Inc.
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

#include "config.h"	/* CONFIG_BOARDDIR */

#ifndef RESET_VECTOR_ADDRESS
#define RESET_VECTOR_ADDRESS	0xfffffffc
#endif

OUTPUT_ARCH(powerpc)
/* Do we need any of these for elf?
   __DYNAMIC = 0;    */
PHDRS
{
  text PT_LOAD;
  bss PT_LOAD;
}

SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = + SIZEOF_HEADERS;
  .interp : { *(.interp) }
  .hash          : { *(.hash)		}
  .dynsym        : { *(.dynsym)		}
  .dynstr        : { *(.dynstr)		}
  .rel.text      : { *(.rel.text)		}
  .rela.text     : { *(.rela.text)	}
  .rel.data      : { *(.rel.data)		}
  .rela.data     : { *(.rela.data)	}
  .rel.rodata    : { *(.rel.rodata)	}
  .rela.rodata   : { *(.rela.rodata)	}
  .rel.got       : { *(.rel.got)		}
  .rela.got      : { *(.rela.got)		}
  .rel.ctors     : { *(.rel.ctors)	}
  .rela.ctors    : { *(.rela.ctors)	}
  .rel.dtors     : { *(.rel.dtors)	}
  .rela.dtors    : { *(.rela.dtors)	}
  .rel.bss       : { *(.rel.bss)		}
  .rela.bss      : { *(.rela.bss)		}
  .rel.plt       : { *(.rel.plt)		}
  .rela.plt      : { *(.rela.plt)		}
  .init          : { *(.init)	}
  .plt : { *(.plt) }
  .text      :
  {
    *(.text)
    *(.got1)
   } :text
    _etext = .;
    PROVIDE (etext = .);
    .rodata    :
   {
    *(.eh_frame)
    *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
  } :text
  .fini      : { *(.fini)    } =0
  .ctors     : { *(.ctors)   }
  .dtors     : { *(.dtors)   }

  /* Read-write section, merged into data segment: */
  . = (. + 0x00FF) & 0xFFFFFF00;
  _erotext = .;
  PROVIDE (erotext = .);
  .reloc   :
  {
    *(.got)
    _GOT2_TABLE_ = .;
    *(.got2)
    _FIXUP_TABLE_ = .;
    *(.fixup)
  }
  __got2_entries = (_FIXUP_TABLE_ - _GOT2_TABLE_) >> 2;
  __fixup_entries = (. - _FIXUP_TABLE_) >> 2;

  .data    :
  {
    *(.data)
    *(.data1)
    *(.sdata)
    *(.sdata2)
    *(.dynamic)
    CONSTRUCTORS
  }
  _edata  =  .;
  PROVIDE (edata = .);

  . = .;
  __u_boot_cmd_start = .;
  .u_boot_cmd : { *(.u_boot_cmd) }
  __u_boot_cmd_end = .;

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

#ifdef CONFIG_440
  .bootpg RESET_VECTOR_ADDRESS - 0xffc :
  {
    arch/powerpc/cpu/ppc4xx/start.o	(.bootpg)

    /*
     * PPC440 board need a board specific object with the
     * TLB definitions. This needs to get included right after
     * start.o, since the first shadow TLB only covers 4k
     * of address space.
     */
    CONFIG_BOARDDIR/init.o	(.bootpg)
  } :text = 0xffff
#endif

  .resetvec RESET_VECTOR_ADDRESS :
  {
    *(.resetvec)
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
   *(.sbss) *(.scommon)
   *(.dynbss)
   *(.bss)
   *(COMMON)
  } :bss

  . = ALIGN(4);
  _end = . ;
  PROVIDE (end = .);
}

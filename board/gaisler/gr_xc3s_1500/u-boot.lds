/* Linker script for Gaisler Research AB's GR-XC3S-1500 board
 * with template design.
 *
 * (C) Copyright 2007
 * Daniel Hellstrom, Gaisler Research, daniel@gaisler.com.
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
 *
 */

OUTPUT_FORMAT("elf32-sparc", "elf32-sparc", "elf32-sparc")
OUTPUT_ARCH(sparc)
ENTRY(_start)
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
	.init          : { *(.init)		}
	.plt : { *(.plt) }

	.text : {
		_load_addr = .;
		_text = .;

		*(.start)
		arch/sparc/cpu/leon3/start.o (.text)
/* 8k is the same as the PROM offset from end of main memory, (CONFIG_SYS_PROM_SIZE) */
		. = ALIGN(8192);
/* PROM CODE, Will be relocated to the end of memory,
 * no global data accesses please.
 */
		__prom_start = .;
		*(.prom.pgt)
		*(.prom.data)
		*(.prom.text)
		. = ALIGN(16);
		__prom_end = .;
		*(.text)
		*(.fixup)
		*(.gnu.warning)
/*		*(.got1)*/
		. = ALIGN(16);
		*(.eh_frame)
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}
	. = ALIGN(4);
	_etext = .;

	/* CMD Table */


	. = ALIGN(4);
	.u_boot_list : {
	#include <u-boot.lst>
	}

	.data	:
	{
		*(.data)
		*(.data1)
		*(.data.rel)
		*(.data.rel.*)
		*(.sdata)
		*(.sdata2)
		*(.dynamic)
		CONSTRUCTORS
	}
	_edata	=	.;
	PROVIDE (edata = .);

	. = ALIGN(4);
	__got_start = .;
	.got : {
		*(.got)
/*		*(.data.rel)
		*(.data.rel.local)*/
		. = ALIGN(16);
	}
	__got_end = .;

/*	.data.rel : { } */


	. = ALIGN(4096);
	__init_begin = .;
	.text.init : { *(.text.init) }
	.data.init : { *(.data.init) }
	. = ALIGN(4096);
	__init_end = .;

	__bss_start = .;
	.bss		:
	{
	 *(.sbss) *(.scommon)
	 *(.dynbss)
	 *(.bss)
	 *(COMMON)
	. = ALIGN(16); /* to speed clearing of bss up */
	}
	__bss_end = . ;
	__bss_end__ = . ;
	PROVIDE (end = .);

/* Relocated into main memory */

	/* Start of main memory */
	/*. = 0x40000000;*/

	.stack (NOLOAD)	: { *(.stack) }

	/* PROM CODE */

	/* global data in RAM passed to kernel after booting */


  .stab 0		: { *(.stab) }
  .stabstr 0		: { *(.stabstr) }
  .stab.excl 0		: { *(.stab.excl) }
  .stab.exclstr 0	: { *(.stab.exclstr) }
  .stab.index 0		: { *(.stab.index) }
  .stab.indexstr 0	: { *(.stab.indexstr) }
  .comment 0		: { *(.comment) }

}

/*
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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

#include <config.h>
OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = CONFIG_SYS_TEXT_BASE;	/* Location of bootcode in flash */
	__text_start = .;
	.text  : { *(.text*); }

	. = ALIGN(4);
	__u_boot_cmd_start = .;
	.u_boot_cmd : { *(.u_boot_cmd) }
	. = ALIGN(4);
	__u_boot_cmd_end = .;

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = ALIGN(4);
	.data : { *(.data*) }

	. = ALIGN(4);
	.dynsym : { *(.dynsym*) }

	. = ALIGN(4);
	.hash : { *(.hash*) }

	. = ALIGN(4);
	.got : { *(.got*) }

	. = ALIGN(4);
	__data_end = .;

	. = ALIGN(4);
	__bss_start = ABSOLUTE(.);
	.bss (NOLOAD) : { *(.bss) }
	. = ALIGN(4);
	__bss_end = ABSOLUTE(.);

	. = ALIGN(4);
	__rel_dyn_start = .;
	.rel.dyn : { *(.rel.dyn) }
	__rel_dyn_end = .;

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }

	/* 16bit realmode trampoline code */
	.realmode REALMODE_BASE : AT ( LOADADDR(.rel.dyn) + SIZEOF(.rel.dyn) ) { KEEP(*(.realmode)) }

	__realmode_start = LOADADDR(.realmode);
	__realmode_size = SIZEOF(.realmode);

	/* 16bit BIOS emulation code (just enough to boot Linux) */
	.bios 0 : AT ( LOADADDR(.realmode) + SIZEOF(.realmode) ) { KEEP(*(.bios)) }

	__bios_start = LOADADDR(.bios);
	__bios_size = SIZEOF(.bios);

	/*
	 * The following expressions place the 16-bit Real-Mode code and
	 * Reset Vector at the end of the Flash ROM
	 */
	. = START_16;
	.start16 : AT (CONFIG_SYS_TEXT_BASE + (CONFIG_SYS_MONITOR_LEN - RESET_SEG_SIZE + START_16)) { KEEP(*(.start16)); }

	. = RESET_VEC_LOC;
	.resetvec : AT (CONFIG_SYS_TEXT_BASE + (CONFIG_SYS_MONITOR_LEN - RESET_SEG_SIZE + RESET_VEC_LOC)) { KEEP(*(.resetvec)); }
}

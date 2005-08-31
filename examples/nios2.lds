/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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


OUTPUT_FORMAT("elf32-littlenios2")
OUTPUT_ARCH(nios2)
ENTRY(_start)

SECTIONS
{
	.text :
	{
	  *(.text)
	  *(.text.*)
	  *(.gnu.linkonce.t*)
	  *(.rodata)
	  *(.rodata.*)
	  *(.gnu.linkonce.r*)
	}
	. = ALIGN (4);
	_etext = .;
	PROVIDE (etext = .);

	/* INIT DATA sections - "Small" data (see the gcc -G option)
	 * is always gp-relative. Here we make all init data sections
	 * adjacent to simplify the startup code -- and provide
	 * the global pointer for gp-relative access.
	 */
	_data = .;
	.data :
	{
	  *(.data)
	  *(.data.*)
	  *(.gnu.linkonce.d*)
	}

	. = ALIGN(16);
	_gp = .;			/* Global pointer addr */
	PROVIDE (gp = .);

	.sdata :
	{
	  *(.sdata)
	  *(.sdata.*)
	  *(.gnu.linkonce.s.*)
	}
	. = ALIGN(4);

	_edata = .;
	PROVIDE (edata = .);

	/* UNINIT DATA - Small uninitialized data is first so it's
	 * adjacent to sdata and can be referenced via gp. The normal
	 * bss follows. We keep it adjacent to simplify init code.
	 */
	__bss_start = .;
	.sbss :
	{
	  *(.sbss)
	  *(.sbss.*)
	  *(.gnu.linkonce.sb.*)
	  *(.scommon)
	}
	. = ALIGN(4);
	.bss :
	{
	  *(.bss)
	  *(.bss.*)
	  *(.dynbss)
	  *(COMMON)
	  *(.scommon)
	}
	. = ALIGN(4);
	_end = .;
	PROVIDE (end = .);

	/* CMD TABLE - uboot command sections
	 */
	. = .;
	__uboot_cmd_start = .;
	.u_boot_cmd :
	{
	  *(.u_boot_cmd)
	}
	. = ALIGN(4);
	__u_boot_cmd_end = .;

	/* DEBUG -- symbol table, string table, etc. etc.
	 */
	.stab 0 : { *(.stab) }
	.stabstr 0 : { *(.stabstr) }
	.stab.excl 0 : { *(.stab.excl) }
	.stab.exclstr 0 : { *(.stab.exclstr) }
	.stab.index 0 : { *(.stab.index) }
	.stab.indexstr 0 : { *(.stab.indexstr) }
	.comment 0 : { *(.comment) }
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }
}

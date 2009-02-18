
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

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	. = 0x387c0000;                     /* Where bootcode in the flash is mapped */
	.text  : { *(.text); }

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }

	. = 0x400000;	                    /* Ram data segment to use */
	_i386boot_romdata_dest = ABSOLUTE(.);
	.data : AT ( LOADADDR(.rodata) + SIZEOF(.rodata) ) { *(.data) }
	_i386boot_romdata_start = LOADADDR(.data);

	. = ALIGN(4);
	.got : AT ( LOADADDR(.data) + SIZEOF(.data) ) { *(.got) }
	_i386boot_romdata_size = SIZEOF(.data) + SIZEOF(.got);


	. = ALIGN(4);
	_i386boot_bss_start = ABSOLUTE(.);
	.bss (NOLOAD) : { *(.bss) . = ALIGN(4); }
	_i386boot_bss_size = SIZEOF(.bss);


  . = .;
  __u_boot_cmd_start = .;
  .u_boot_cmd : { *(.u_boot_cmd) }
  __u_boot_cmd_end = .;


	/* 16bit realmode trampoline code */
	.realmode 0x7c0 : AT ( LOADADDR(.got) + SIZEOF(.got) ) { *(.realmode) }

	_i386boot_realmode = LOADADDR(.realmode);
	_i386boot_realmode_size = SIZEOF(.realmode);

	/* 16bit BIOS emulation code (just enough to boot Linux) */
	.bios 0 : AT ( LOADADDR(.realmode) + SIZEOF(.realmode) ) { *(.bios) }

	_i386boot_bios = LOADADDR(.bios);
	_i386boot_bios_size = SIZEOF(.bios);


	/* The load addresses below assumes that the flash
	 * will be mapped so that 0x387f0000 == 0xffff0000
	 * at reset time
	 *
	 * The fe00 and ff00 offsets of the start32 and start16
	 * segments are arbitrary, the just have to be mapped
	 * at reset and the code have to fit.
	 * The fff0 offset of resetvec is important, however.
	 */


	. = 0xfffffe00;
	.start32 : AT (0x387ffe00) { *(.start32); }

	. = 0xff00;
	.start16 : AT (0x387fff00) { *(.start16); }

	. = 0xfff0;
	.resetvec : AT (0x387ffff0) { *(.resetvec); }
	_i386boot_end = (LOADADDR(.resetvec) + SIZEOF(.resetvec) );
}

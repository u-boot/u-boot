/*
 * Copyrigth (c) 2007
 * Yoshihiro Shimoda <shimoda.yoshihiro@renesas.com>
 *
 * Copyrigth (c) 2007
 * Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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

OUTPUT_FORMAT("elf32-sh-linux", "elf32-sh-linux", "elf32-sh-linux")
OUTPUT_ARCH(sh)
ENTRY(_start)

SECTIONS
{
	/*
	   Base address of internal SDRAM is 0x0C000000.
	   Although size of SDRAM can be either 16 or 32 MBytes,
	   we assume 16 MBytes (ie ignore upper half if the full
	   32 MBytes is present).

	   NOTE: This address must match with the definition of
	   TEXT_BASE in config.mk (in this directory).

	*/
	. = 0x8C000000 + (64*1024*1024) - (256*1024);

	PROVIDE (reloc_dst = .);

	PROVIDE (_ftext = .);
	PROVIDE (_fcode = .);
	PROVIDE (_start = .);

	.text :
	{
		cpu/sh3/start.o		(.text)
		. = ALIGN(8192);
		common/environment.o	(.ppcenv)
		. = ALIGN(8192);
		common/environment.o	(.ppcenvr)
		. = ALIGN(8192);
		*(.text)
		. = ALIGN(4);
	} =0xFF
	PROVIDE (_ecode = .);
	.rodata :
	{
		*(.rodata)
		. = ALIGN(4);
	}
	PROVIDE (_etext = .);


	PROVIDE (_fdata = .);
	.data :
	{
		*(.data)
		. = ALIGN(4);
	}
	PROVIDE (_edata = .);

	PROVIDE (_fgot = .);
	.got :
	{
		*(.got)
		. = ALIGN(4);
	}
	PROVIDE (_egot = .);

	PROVIDE (__u_boot_cmd_start = .);
	.u_boot_cmd :
	{
		*(.u_boot_cmd)
		. = ALIGN(4);
	}
	PROVIDE (__u_boot_cmd_end = .);

	PROVIDE (reloc_dst_end = .);
	/* _reloc_dst_end = .; */

	PROVIDE (bss_start = .);
	PROVIDE (__bss_start = .);
	.bss :
	{
		*(.bss)
		. = ALIGN(4);
	}
	PROVIDE (bss_end = .);

	PROVIDE (_end = .);
}

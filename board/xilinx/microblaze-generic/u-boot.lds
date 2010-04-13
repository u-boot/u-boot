/*
 * (C) Copyright 2004 Atmark Techno, Inc.
 *
 * Yasushi SHOJI <yashi@atmark-techno.com>
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

OUTPUT_ARCH(microblaze)
ENTRY(_start)

SECTIONS
{
	.text ALIGN(0x4):
	{
		__text_start = .;
		arch/microblaze/cpu/start.o (.text)
		*(.text)
		__text_end = .;
	}

	.rodata ALIGN(0x4):
	{
		__rodata_start = .;
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
		__rodata_end = .;
	}

	.data ALIGN(0x4):
	{
		__data_start = .;
		*(.data)
		__data_end = .;
	}

	.u_boot_cmd ALIGN(0x4):
	{
		. = .;
		__u_boot_cmd_start = .;
		*(.u_boot_cmd)
		__u_boot_cmd_end = .;
	}

	.bss ALIGN(0x4):
	{
		__bss_start = .;
		*(.sbss)
		*(.scommon)
		*(.bss)
		*(COMMON)
		. = ALIGN(4);
		__bss_end = .;
	}
	__end = . ;
}

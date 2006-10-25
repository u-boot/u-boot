/* -*- Fundamental -*-
 *
 * Copyright (C) 2005-2006 Atmel Corporation
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
OUTPUT_FORMAT("elf32-avr32", "elf32-avr32", "elf32-avr32")
OUTPUT_ARCH(avr32)
ENTRY(_start)

SECTIONS
{
	. = 0;
	_text = .;
	.text : {
		*(.text)
		*(.text.*)
	}

	. = ALIGN(32);
	__flashprog_start = .;
	.flashprog : {
		*(.flashprog)
	}
	. = ALIGN(32);
	__flashprog_end = .;

	. = ALIGN(8);
	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}
	_etext = .;

	__data_lma = ALIGN(8);
	. = 0x24000000;
	_data = .;
	.data : AT(__data_lma) {
		*(.data)
		*(.data.*)
	}

	. = ALIGN(4);
	__u_boot_cmd_start = .;
	__u_boot_cmd_lma = __data_lma + (__u_boot_cmd_start - _data);
	.u_boot_cmd : AT(__u_boot_cmd_lma) {
		KEEP(*(.u_boot_cmd))
	}
	__u_boot_cmd_end = .;

	. = ALIGN(8);
	_edata = .;
	__edata_lma = __u_boot_cmd_lma + (_edata - __u_boot_cmd_start);

	.bss : AT(__edata_lma) {
		*(.bss)
		*(.bss.*)
	}
	. = ALIGN(8);
	_end = .;
}

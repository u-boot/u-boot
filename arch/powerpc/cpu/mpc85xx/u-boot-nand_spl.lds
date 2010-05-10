/*
 * (C) Copyright 2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de
 *
 * Copyright 2009 Freescale Semiconductor, Inc.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
	. = 0xfff00000;
	.text : {
		*(.text)
	}
	_etext = .;

	.reloc : {
		_GOT2_TABLE_ = .;
		*(.got2)
		_FIXUP_TABLE_ = .;
		*(.fixup)
	}
	__got2_entries = (_FIXUP_TABLE_ - _GOT2_TABLE_) >> 2;
	__fixup_entries = (. - _FIXUP_TABLE_) >> 2;

	. = ALIGN(8);
	.data : {
		*(.rodata*)
		*(.data*)
		*(.sdata*)
	}
	_edata  =  .;

	. = ALIGN(8);
	__init_begin = .;
	__init_end = .;

	.resetvec ADDR(.text) + 0xffc : {
		*(.resetvec)
	} = 0xffff

	__bss_start = .;
	.bss : {
		*(.sbss)
		*(.bss)
	}
	_end = .;
}
ASSERT(__init_end <= 0xfff00ffc, "NAND bootstrap too big");

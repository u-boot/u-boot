/*
 * Copyright 2012-2013 Stefan Roese <sr@denx.de>
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

MEMORY
{
	sdram : ORIGIN = CONFIG_SPL_BSS_START_ADDR,
		LENGTH = CONFIG_SPL_BSS_MAX_SIZE
	flash : ORIGIN = CONFIG_SPL_TEXT_BASE,
		LENGTH = CONFIG_SYS_SPL_MAX_LEN
}

OUTPUT_ARCH(powerpc)
ENTRY(_start)
SECTIONS
{
#ifdef CONFIG_440
	.bootpg 0xfffff000 :
	{
		arch/powerpc/cpu/ppc4xx/start.o	(.bootpg)

		/*
		 * PPC440 board need a board specific object with the
		 * TLB definitions. This needs to get included right after
		 * start.o, since the first shadow TLB only covers 4k
		 * of address space.
		 */
		CONFIG_BOARDDIR/init.o	(.bootpg)
	} > flash
#endif

	.resetvec 0xFFFFFFFC :
	{
		KEEP(*(.resetvec))
	} > flash

	.text :
	{
		__start = .;
		arch/powerpc/cpu/ppc4xx/start.o	(.text)
		CONFIG_BOARDDIR/init.o	(.text)
		*(.text*)
	} > flash

	. = ALIGN(4);
	.data : { *(SORT_BY_ALIGNMENT(.data*)) } > flash

	. = ALIGN(4);
	.rodata : { *(SORT_BY_ALIGNMENT(.rodata*)) } > flash

	.bss :
	{
		. = ALIGN(4);
		__bss_start = .;
		*(.bss*)
		. = ALIGN(4);
		__bss_end = .;
	} > sdram
}

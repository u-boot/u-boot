/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

OUTPUT_FORMAT ("elf32-bigarm", "elf32-bigarm", "elf32-bigarm")
OUTPUT_ARCH (arm)
ENTRY (_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN (4);
	.text : {
		arch/arm/cpu/ixp/start.o(.text*)
		net/libnet.o(.text*)
		board/dvlhost/libdvlhost.o(.text*)
		arch/arm/cpu/ixp/libixp.o(.text*)
		drivers/serial/libserial.o(.text*)

		. = env_offset;
		common/env_embedded.o(.ppcenv)
		*(.text*)
	}

	. = ALIGN (4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}
	. = ALIGN (4);
	.data : {
		*(.data*)
	}
	. = ALIGN (4);
	.got : {
		*(.got)
	}
	. =.;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN (4);
	.rel.dyn : {
		__rel_dyn_start = .;
		*(.rel*)
		__rel_dyn_end = .;
	}

	.dynsym : {
		__dynsym_start = .;
		*(.dynsym)
	}

	_end = .;

	.bss_start __rel_dyn_start (OVERLAY) : {
		KEEP(*(.__bss_start));
	}

	.bss __bss_start (OVERLAY) : {
		*(.bss*)
		 . = ALIGN(4);
		 __bss_end = .;
	}
	.bss_end __bss_end (OVERLAY) : {
		KEEP(*(__bss_end));
	}

	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}

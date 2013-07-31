/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

OUTPUT_FORMAT ("elf32-bigarm", "elf32-bigarm", "elf32-bigarm")
OUTPUT_ARCH (arm)
ENTRY (_start)
SECTIONS
{
	. = 0x00000000;

	. = ALIGN (4);
	.text : {
		*(.__image_copy_start)
		arch/arm/cpu/ixp/start.o(.text*)
		net/libnet.o(.text*)
		board/actux3/libactux3.o(.text*)
		arch/arm/cpu/ixp/libixp.o(.text*)
		drivers/input/libinput.o(.text*)

		. = env_offset;
		common/env_embedded.o(.ppcenv)
		*(.text*)
	}

	. = ALIGN(4);
	.rodata : {
		*(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*)))
	}
	. = ALIGN(4);
	.data : {
		*(.data*)
	}
	. = ALIGN(4);
	.got : {
		*(.got)
	}
	. =.;

	. = ALIGN(4);
	.u_boot_list : {
		KEEP(*(SORT(.u_boot_list*)));
	}

	. = ALIGN (4);

	.image_copy_end :
	{
		*(.__image_copy_end)
	}

	.rel_dyn_start :
	{
		*(.__rel_dyn_start)
	}

	.rel.dyn : {
		*(.rel*)
	}

	.rel_dyn_end :
	{
		*(.__rel_dyn_end)
	}

	_end = .;

/*
 * Compiler-generated __bss_start and __bss_end, see arch/arm/lib/bss.c
 * __bss_base and __bss_limit are for linker only (overlay ordering)
 */

	.bss_start __rel_dyn_start (OVERLAY) : {
		KEEP(*(.__bss_start));
		__bss_base = .;
	}

	.bss __bss_base (OVERLAY) : {
		*(.bss*)
		 . = ALIGN(4);
		__bss_limit = .;
	}
	.bss_end __bss_limit (OVERLAY) : {
		KEEP(*(.__bss_end));
	}

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}

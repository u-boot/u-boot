/*
 * (C) Copyright 2008-2011
 * Graeme Russ, <graeme.russ@gmail.com>
 *
 * (C) Copyright 2002
 * Daniel Engström, Omicron Ceti AB, <daniel@omicron.se>
 *
 * (C) Copyright 2002
 * Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <inttypes.h>
#include <asm/u-boot-x86.h>
#include <asm/relocate.h>
#include <asm/sections.h>
#include <elf.h>

DECLARE_GLOBAL_DATA_PTR;

int copy_uboot_to_ram(void)
{
	size_t len = (size_t)&__data_end - (size_t)&__text_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	memcpy((void *)gd->relocaddr, (void *)&__text_start, len);

	return 0;
}

int clear_bss(void)
{
	ulong dst_addr = (ulong)&__bss_start + gd->reloc_off;
	size_t len = (size_t)&__bss_end - (size_t)&__bss_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	memset((void *)dst_addr, 0x00, len);

	return 0;
}

/*
 * This function has more error checking than you might expect. Please see
 * the commit message for more informaiton.
 */
int do_elf_reloc_fixups(void)
{
	Elf32_Rel *re_src = (Elf32_Rel *)(&__rel_dyn_start);
	Elf32_Rel *re_end = (Elf32_Rel *)(&__rel_dyn_end);

	Elf32_Addr *offset_ptr_rom, *last_offset = NULL;
	Elf32_Addr *offset_ptr_ram;
	unsigned int text_base = 0;

	/* The size of the region of u-boot that runs out of RAM. */
	uintptr_t size = (uintptr_t)&__bss_end - (uintptr_t)&__text_start;

	if (gd->flags & GD_FLG_SKIP_RELOC)
		return 0;
	if (re_src == re_end)
		panic("No relocation data");

#ifdef CONFIG_SYS_TEXT_BASE
	text_base = CONFIG_SYS_TEXT_BASE;
#else
	panic("No CONFIG_SYS_TEXT_BASE");
#endif
	do {
		/* Get the location from the relocation entry */
		offset_ptr_rom = (Elf32_Addr *)re_src->r_offset;

		/* Check that the location of the relocation is in .text */
		if (offset_ptr_rom >= (Elf32_Addr *)text_base &&
		    offset_ptr_rom > last_offset) {

			/* Switch to the in-RAM version */
			offset_ptr_ram = (Elf32_Addr *)((ulong)offset_ptr_rom +
							gd->reloc_off);

			/* Check that the target points into .text */
			if (*offset_ptr_ram >= text_base &&
			    *offset_ptr_ram <= text_base + size) {
				*offset_ptr_ram += gd->reloc_off;
			} else {
				debug("   %p: rom reloc %x, ram %p, value %x,"
					" limit %" PRIXPTR "\n", re_src,
					re_src->r_offset, offset_ptr_ram,
					*offset_ptr_ram,
					text_base + size);
			}
		} else {
			debug("   %p: rom reloc %x, last %p\n", re_src,
			       re_src->r_offset, last_offset);
		}
		last_offset = offset_ptr_rom;

	} while (++re_src < re_end);

	return 0;
}

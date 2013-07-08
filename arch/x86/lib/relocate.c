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
#include <libfdt.h>
#include <malloc.h>
#include <asm/u-boot-x86.h>
#include <asm/relocate.h>
#include <asm/sections.h>
#include <elf.h>

DECLARE_GLOBAL_DATA_PTR;

int copy_uboot_to_ram(void)
{
	size_t len = (size_t)&__data_end - (size_t)&__text_start;

	memcpy((void *)gd->relocaddr, (void *)&__text_start, len);

	return 0;
}

int copy_fdt_to_ram(void)
{
	if (gd->new_fdt) {
		ulong fdt_size;

		fdt_size = ALIGN(fdt_totalsize(gd->fdt_blob) + 0x1000, 32);

		memcpy(gd->new_fdt, gd->fdt_blob, fdt_size);
		debug("Relocated fdt from %p to %p, size %lx\n",
		       gd->fdt_blob, gd->new_fdt, fdt_size);
		gd->fdt_blob = gd->new_fdt;
	}

	return 0;
}

int clear_bss(void)
{
	ulong dst_addr = (ulong)&__bss_start + gd->reloc_off;
	size_t len = (size_t)&__bss_end - (size_t)&__bss_start;

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

	/* The size of the region of u-boot that runs out of RAM. */
	uintptr_t size = (uintptr_t)&__bss_end - (uintptr_t)&__text_start;

	do {
		/* Get the location from the relocation entry */
		offset_ptr_rom = (Elf32_Addr *)re_src->r_offset;

		/* Check that the location of the relocation is in .text */
		if (offset_ptr_rom >= (Elf32_Addr *)CONFIG_SYS_TEXT_BASE &&
				offset_ptr_rom > last_offset) {

			/* Switch to the in-RAM version */
			offset_ptr_ram = (Elf32_Addr *)((ulong)offset_ptr_rom +
							gd->reloc_off);

			/* Check that the target points into .text */
			if (*offset_ptr_ram >= CONFIG_SYS_TEXT_BASE &&
					*offset_ptr_ram <=
					(CONFIG_SYS_TEXT_BASE + size)) {
				*offset_ptr_ram += gd->reloc_off;
			} else {
				debug("   %p: rom reloc %x, ram %p, value %x,"
					" limit %lx\n", re_src,
					re_src->r_offset, offset_ptr_ram,
					*offset_ptr_ram,
					CONFIG_SYS_TEXT_BASE + size);
			}
		} else {
			debug("   %p: rom reloc %x, last %p\n", re_src,
			       re_src->r_offset, last_offset);
		}
		last_offset = offset_ptr_rom;

	} while (++re_src < re_end);

	return 0;
}

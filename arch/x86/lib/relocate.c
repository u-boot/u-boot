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

#include <common.h>
#include <malloc.h>
#include <asm/u-boot-x86.h>
#include <asm/relocate.h>
#include <elf.h>

int copy_uboot_to_ram(void)
{
	size_t len = (size_t)&__data_end - (size_t)&__text_start;

	memcpy((void *)gd->relocaddr, (void *)&__text_start, len);

	return 0;
}

int clear_bss(void)
{
	ulong dst_addr = (ulong)&__bss_start + gd->reloc_off;
	size_t len = (size_t)&__bss_end - (size_t)&__bss_start;

	memset((void *)dst_addr, 0x00, len);

	return 0;
}

int do_elf_reloc_fixups(void)
{
	Elf32_Rel *re_src = (Elf32_Rel *)(&__rel_dyn_start);
	Elf32_Rel *re_end = (Elf32_Rel *)(&__rel_dyn_end);

	Elf32_Addr *offset_ptr_rom;
	Elf32_Addr *offset_ptr_ram;

	/* The size of the region of u-boot that runs out of RAM. */
	uintptr_t size = (uintptr_t)&__bss_end - (uintptr_t)&__text_start;

	do {
		/* Get the location from the relocation entry */
		offset_ptr_rom = (Elf32_Addr *)re_src->r_offset;

		/* Check that the location of the relocation is in .text */
		if (offset_ptr_rom >= (Elf32_Addr *)CONFIG_SYS_TEXT_BASE) {

			/* Switch to the in-RAM version */
			offset_ptr_ram = (Elf32_Addr *)((ulong)offset_ptr_rom +
							gd->reloc_off);

			/* Check that the target points into .text */
			if (*offset_ptr_ram >= CONFIG_SYS_TEXT_BASE &&
					*offset_ptr_ram <=
					(CONFIG_SYS_TEXT_BASE + size)) {
				*offset_ptr_ram += gd->reloc_off;
			}
		}
	} while (++re_src < re_end);

	return 0;
}

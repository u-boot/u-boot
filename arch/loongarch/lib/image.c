// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 *
 * Based on riscv/lib/image.c
 */

#include <asm/global_data.h>
#include <errno.h>
#include <image.h>
#include <linux/bitfield.h>
#include <linux/sizes.h>
#include <linux/stddef.h>
#include <mapmem.h>

DECLARE_GLOBAL_DATA_PTR;

#define LINUX_LOONGARCH_IMAGE_MAGIC	0x818223cd

struct linux_image_h {
	uint32_t	code0;			/* Executable code */
	uint32_t	code1;			/* Executable code */
	uint64_t	kernel_entry;		/* Kernel entry point */
	uint64_t	image_size;		/* Effective Image size */
	uint64_t	load_offset;		/* load offset */
	uint64_t	res1;			/* reserved */
	uint64_t	res2;			/* reserved */
	uint64_t	res3;			/* reserved */
	uint32_t	magic;			/* Magic number */
	uint32_t	pe_header;		/* Offset to the PE header */
};

int booti_setup(ulong image, ulong *relocated_addr, ulong *size, ulong *ep,
		bool force_reloc)
{
	struct linux_image_h *lhdr;
	phys_addr_t ep_phys;

	lhdr = (struct linux_image_h *)map_sysmem(image, 0);

	if (lhdr->magic != LINUX_LOONGARCH_IMAGE_MAGIC) {
		puts("Bad Linux LoongArch Image magic!\n");
		return -EINVAL;
	}

	if (lhdr->image_size == 0) {
		puts("Image lacks image_size field, error!\n");
		return -EINVAL;
	}

	*size = lhdr->image_size;
	if (force_reloc ||
	    (gd->ram_base <= image && image < gd->ram_base + gd->ram_size)) {
		*relocated_addr = gd->ram_base + lhdr->load_offset;
	} else {
		*relocated_addr = image;
	}

	/*
	 * Before Linux commit beb2800074c1 ("LoongArch: Fix entry point in
	 * kernel image header"), kernel_entry is filled with a DMW-mapped
	 * virtual address instead of a physical address, we must convert it
	 * back to physical address.
	 *
	 * On loongarch64, the lowest 60 bits of DMW-mapped address are
	 * identical to the physical address, and the highest 4 bits are for
	 * determining access privileges and types. So clearing the highest 4
	 * bits works around the issue, and is a no-op since the length of
	 * physical addresses never exceed 60 bits.
	 */
	ep_phys = lhdr->kernel_entry & GENMASK(59, 0);
	*ep = *relocated_addr + (ep_phys - lhdr->load_offset);

	unmap_sysmem(lhdr);

	return 0;
}

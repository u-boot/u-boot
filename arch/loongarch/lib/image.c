// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>
 *
 * Based on riscv/lib/image.c
 */

#include <image.h>
#include <mapmem.h>
#include <errno.h>
#include <asm/global_data.h>
#include <linux/sizes.h>
#include <linux/stddef.h>
#include <asm/addrspace.h>

DECLARE_GLOBAL_DATA_PTR;

#define LINUX_LOONGARCH_IMAGE_MAGIC	0x818223cd

struct linux_image_h {
	uint32_t	code0;			/* Executable code */
	uint32_t	code1;			/* Executable code */
	uint64_t	kernel_entry;	/* Kernel entry point */
	uint64_t	image_size;		/* Effective Image size */
	uint64_t	load_offset;	/* load offset */
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

	/* To workaround kernel supplying DMW based virtual address */
	ep_phys = TO_PHYS(lhdr->kernel_entry);
	*ep = *relocated_addr + (ep_phys - lhdr->load_offset);

	unmap_sysmem(lhdr);

	return 0;
}

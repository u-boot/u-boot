// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <dm.h>
#include <elf.h>
#include <remoteproc.h>

/* Basic function to verify ELF32 image format */
int rproc_elf32_sanity_check(ulong addr, ulong size)
{
	Elf32_Ehdr *ehdr;
	char class;

	if (!addr) {
		pr_debug("Invalid fw address?\n");
		return -EFAULT;
	}

	if (size < sizeof(Elf32_Ehdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	ehdr = (Elf32_Ehdr *)addr;
	class = ehdr->e_ident[EI_CLASS];

	if (!IS_ELF(*ehdr) || ehdr->e_type != ET_EXEC || class != ELFCLASS32) {
		pr_debug("Not an executable ELF32 image\n");
		return -EPROTONOSUPPORT;
	}

	/* We assume the firmware has the same endianness as the host */
# ifdef __LITTLE_ENDIAN
	if (ehdr->e_ident[EI_DATA] != ELFDATA2LSB) {
# else /* BIG ENDIAN */
	if (ehdr->e_ident[EI_DATA] != ELFDATA2MSB) {
# endif
		pr_debug("Unsupported firmware endianness\n");
		return -EILSEQ;
	}

	if (size < ehdr->e_shoff + sizeof(Elf32_Shdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	if (memcmp(ehdr->e_ident, ELFMAG, SELFMAG)) {
		pr_debug("Image is corrupted (bad magic)\n");
		return -EBADF;
	}

	if (ehdr->e_phnum == 0) {
		pr_debug("No loadable segments\n");
		return -ENOEXEC;
	}

	if (ehdr->e_phoff > size) {
		pr_debug("Firmware size is too small\n");
		return -ENOSPC;
	}

	return 0;
}

/* A very simple elf loader, assumes the image is valid */
int rproc_elf32_load_image(struct udevice *dev, unsigned long addr)
{
	Elf32_Ehdr *ehdr; /* Elf header structure pointer */
	Elf32_Phdr *phdr; /* Program header structure pointer */
	const struct dm_rproc_ops *ops;
	unsigned int i;

	ehdr = (Elf32_Ehdr *)addr;
	phdr = (Elf32_Phdr *)(addr + ehdr->e_phoff);

	ops = rproc_get_ops(dev);

	/* Load each program header */
	for (i = 0; i < ehdr->e_phnum; ++i) {
		void *dst = (void *)(uintptr_t)phdr->p_paddr;
		void *src = (void *)addr + phdr->p_offset;

		if (phdr->p_type != PT_LOAD)
			continue;

		if (ops->device_to_virt)
			dst = ops->device_to_virt(dev, (ulong)dst);

		dev_dbg(dev, "Loading phdr %i to 0x%p (%i bytes)\n",
			i, dst, phdr->p_filesz);
		if (phdr->p_filesz)
			memcpy(dst, src, phdr->p_filesz);
		if (phdr->p_filesz != phdr->p_memsz)
			memset(dst + phdr->p_filesz, 0x00,
			       phdr->p_memsz - phdr->p_filesz);
		flush_cache(rounddown((unsigned long)dst, ARCH_DMA_MINALIGN),
			    roundup((unsigned long)dst + phdr->p_filesz,
				    ARCH_DMA_MINALIGN) -
			    rounddown((unsigned long)dst, ARCH_DMA_MINALIGN));
		++phdr;
	}

	return 0;
}

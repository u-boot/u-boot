// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2019, STMicroelectronics - All Rights Reserved
 */
#include <common.h>
#include <cpu_func.h>
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

/* Basic function to verify ELF64 image format */
int rproc_elf64_sanity_check(ulong addr, ulong size)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)addr;
	char class;

	if (!addr) {
		pr_debug("Invalid fw address?\n");
		return -EFAULT;
	}

	if (size < sizeof(Elf64_Ehdr)) {
		pr_debug("Image is too small\n");
		return -ENOSPC;
	}

	class = ehdr->e_ident[EI_CLASS];

	if (!IS_ELF(*ehdr) || ehdr->e_type != ET_EXEC || class != ELFCLASS64) {
		pr_debug("Not an executable ELF64 image\n");
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

	if (size < ehdr->e_shoff + sizeof(Elf64_Shdr)) {
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

/* Basic function to verify ELF image format */
int rproc_elf_sanity_check(ulong addr, ulong size)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	if (!addr) {
		dev_err(dev, "Invalid firmware address\n");
		return -EFAULT;
	}

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return rproc_elf64_sanity_check(addr, size);
	else
		return rproc_elf32_sanity_check(addr, size);
}

int rproc_elf32_load_image(struct udevice *dev, unsigned long addr, ulong size)
{
	Elf32_Ehdr *ehdr; /* Elf header structure pointer */
	Elf32_Phdr *phdr; /* Program header structure pointer */
	const struct dm_rproc_ops *ops;
	unsigned int i, ret;

	ret =  rproc_elf32_sanity_check(addr, size);
	if (ret) {
		dev_err(dev, "Invalid ELF32 Image %d\n", ret);
		return ret;
	}

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
			dst = ops->device_to_virt(dev, (ulong)dst,
						  phdr->p_memsz);

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

int rproc_elf64_load_image(struct udevice *dev, ulong addr, ulong size)
{
	const struct dm_rproc_ops *ops = rproc_get_ops(dev);
	u64 da, memsz, filesz, offset;
	Elf64_Ehdr *ehdr;
	Elf64_Phdr *phdr;
	int i, ret = 0;
	void *ptr;

	dev_dbg(dev, "%s: addr = 0x%lx size = 0x%lx\n", __func__, addr, size);

	if (rproc_elf64_sanity_check(addr, size))
		return -EINVAL;

	ehdr = (Elf64_Ehdr *)addr;
	phdr = (Elf64_Phdr *)(addr + (ulong)ehdr->e_phoff);

	/* go through the available ELF segments */
	for (i = 0; i < ehdr->e_phnum; i++, phdr++) {
		da = phdr->p_paddr;
		memsz = phdr->p_memsz;
		filesz = phdr->p_filesz;
		offset = phdr->p_offset;

		if (phdr->p_type != PT_LOAD)
			continue;

		dev_dbg(dev, "%s:phdr: type %d da 0x%llx memsz 0x%llx filesz 0x%llx\n",
			__func__, phdr->p_type, da, memsz, filesz);

		ptr = (void *)(uintptr_t)da;
		if (ops->device_to_virt) {
			ptr = ops->device_to_virt(dev, da, phdr->p_memsz);
			if (!ptr) {
				dev_err(dev, "bad da 0x%llx mem 0x%llx\n", da,
					memsz);
				ret = -EINVAL;
				break;
			}
		}

		if (filesz)
			memcpy(ptr, (void *)addr + offset, filesz);
		if (filesz != memsz)
			memset(ptr + filesz, 0x00, memsz - filesz);

		flush_cache(rounddown((ulong)ptr, ARCH_DMA_MINALIGN),
			    roundup((ulong)ptr + filesz, ARCH_DMA_MINALIGN) -
			    rounddown((ulong)ptr, ARCH_DMA_MINALIGN));
	}

	return ret;
}

int rproc_elf_load_image(struct udevice *dev, ulong addr, ulong size)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	if (!addr) {
		dev_err(dev, "Invalid firmware address\n");
		return -EFAULT;
	}

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return rproc_elf64_load_image(dev, addr, size);
	else
		return rproc_elf32_load_image(dev, addr, size);
}

static ulong rproc_elf32_get_boot_addr(ulong addr)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	return ehdr->e_entry;
}

static ulong rproc_elf64_get_boot_addr(ulong addr)
{
	Elf64_Ehdr *ehdr = (Elf64_Ehdr *)addr;

	return ehdr->e_entry;
}

ulong rproc_elf_get_boot_addr(struct udevice *dev, ulong addr)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)addr;

	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64)
		return rproc_elf64_get_boot_addr(addr);
	else
		return rproc_elf32_get_boot_addr(addr);
}

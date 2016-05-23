/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <errno.h>
#include <malloc.h>
#include <qemu_fw_cfg.h>
#include <asm/io.h>
#include <asm/tables.h>
#include <asm/e820.h>
#include <linux/list.h>
#include <memalign.h>

/*
 * This function allocates memory for ACPI tables
 *
 * @entry : BIOS linker command entry which tells where to allocate memory
 *          (either high memory or low memory)
 * @addr  : The address that should be used for low memory allcation. If the
 *          memory allocation request is 'ZONE_HIGH' then this parameter will
 *          be ignored.
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_allocate(struct bios_linker_entry *entry, u32 *addr)
{
	uint32_t size, align;
	struct fw_file *file;
	unsigned long aligned_addr;

	align = le32_to_cpu(entry->alloc.align);
	/* align must be power of 2 */
	if (align & (align - 1)) {
		printf("error: wrong alignment %u\n", align);
		return -EINVAL;
	}

	file = qemu_fwcfg_find_file(entry->alloc.file);
	if (!file) {
		printf("error: can't find file %s\n", entry->alloc.file);
		return -ENOENT;
	}

	size = be32_to_cpu(file->cfg.size);

	/*
	 * ZONE_HIGH means we need to allocate from high memory, since
	 * malloc space is already at the end of RAM, so we directly use it.
	 * If allocation zone is ZONE_FSEG, then we use the 'addr' passed
	 * in which is low memory
	 */
	if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_HIGH) {
		aligned_addr = (unsigned long)memalign(align, size);
		if (!aligned_addr) {
			printf("error: allocating resource\n");
			return -ENOMEM;
		}
	} else if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG) {
		aligned_addr = ALIGN(*addr, align);
	} else {
		printf("error: invalid allocation zone\n");
		return -EINVAL;
	}

	debug("bios_linker_allocate: allocate file %s, size %u, zone %d, align %u, addr 0x%lx\n",
	      file->cfg.name, size, entry->alloc.zone, align, aligned_addr);

	qemu_fwcfg_read_entry(be16_to_cpu(file->cfg.select),
			      size, (void *)aligned_addr);
	file->addr = aligned_addr;

	/* adjust address for low memory allocation */
	if (entry->alloc.zone == BIOS_LINKER_LOADER_ALLOC_ZONE_FSEG)
		*addr = (aligned_addr + size);

	return 0;
}

/*
 * This function patches ACPI tables previously loaded
 * by bios_linker_allocate()
 *
 * @entry : BIOS linker command entry which tells how to patch
 *          ACPI tables
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_add_pointer(struct bios_linker_entry *entry)
{
	struct fw_file *dest, *src;
	uint32_t offset = le32_to_cpu(entry->pointer.offset);
	uint64_t pointer = 0;

	dest = qemu_fwcfg_find_file(entry->pointer.dest_file);
	if (!dest || !dest->addr)
		return -ENOENT;
	src = qemu_fwcfg_find_file(entry->pointer.src_file);
	if (!src || !src->addr)
		return -ENOENT;

	debug("bios_linker_add_pointer: dest->addr 0x%lx, src->addr 0x%lx, offset 0x%x size %u, 0x%llx\n",
	      dest->addr, src->addr, offset, entry->pointer.size, pointer);

	memcpy(&pointer, (char *)dest->addr + offset, entry->pointer.size);
	pointer	= le64_to_cpu(pointer);
	pointer += (unsigned long)src->addr;
	pointer	= cpu_to_le64(pointer);
	memcpy((char *)dest->addr + offset, &pointer, entry->pointer.size);

	return 0;
}

/*
 * This function updates checksum fields of ACPI tables previously loaded
 * by bios_linker_allocate()
 *
 * @entry : BIOS linker command entry which tells where to update ACPI table
 *          checksums
 * @return: 0 on success, or negative value on failure
 */
static int bios_linker_add_checksum(struct bios_linker_entry *entry)
{
	struct fw_file *file;
	uint8_t *data, cksum = 0;
	uint8_t *cksum_start;

	file = qemu_fwcfg_find_file(entry->cksum.file);
	if (!file || !file->addr)
		return -ENOENT;

	data = (uint8_t *)(file->addr + le32_to_cpu(entry->cksum.offset));
	cksum_start = (uint8_t *)(file->addr + le32_to_cpu(entry->cksum.start));
	cksum = table_compute_checksum(cksum_start,
				       le32_to_cpu(entry->cksum.length));
	*data = cksum;

	return 0;
}

unsigned install_e820_map(unsigned max_entries, struct e820entry *entries)
{
	entries[0].addr = 0;
	entries[0].size = ISA_START_ADDRESS;
	entries[0].type = E820_RAM;

	entries[1].addr = ISA_START_ADDRESS;
	entries[1].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[1].type = E820_RESERVED;

	/*
	 * since we use memalign(malloc) to allocate high memory for
	 * storing ACPI tables, we need to reserve them in e820 tables,
	 * otherwise kernel will reclaim them and data will be corrupted
	 */
	entries[2].addr = ISA_END_ADDRESS;
	entries[2].size = gd->relocaddr - TOTAL_MALLOC_LEN - ISA_END_ADDRESS;
	entries[2].type = E820_RAM;

	/* for simplicity, reserve entire malloc space */
	entries[3].addr = gd->relocaddr - TOTAL_MALLOC_LEN;
	entries[3].size = TOTAL_MALLOC_LEN;
	entries[3].type = E820_RESERVED;

	entries[4].addr = gd->relocaddr;
	entries[4].size = gd->ram_size - gd->relocaddr;
	entries[4].type = E820_RESERVED;

	entries[5].addr = CONFIG_PCIE_ECAM_BASE;
	entries[5].size = CONFIG_PCIE_ECAM_SIZE;
	entries[5].type = E820_RESERVED;

	return 6;
}

/* This function loads and patches ACPI tables provided by QEMU */
u32 write_acpi_tables(u32 addr)
{
	int i, ret = 0;
	struct fw_file *file;
	struct bios_linker_entry *table_loader;
	struct bios_linker_entry *entry;
	uint32_t size;

	/* make sure fw_list is loaded */
	ret = qemu_fwcfg_read_firmware_list();
	if (ret) {
		printf("error: can't read firmware file list\n");
		return addr;
	}

	file = qemu_fwcfg_find_file("etc/table-loader");
	if (!file) {
		printf("error: can't find etc/table-loader\n");
		return addr;
	}

	size = be32_to_cpu(file->cfg.size);
	if ((size % sizeof(*entry)) != 0) {
		printf("error: table-loader maybe corrupted\n");
		return addr;
	}

	table_loader = malloc(size);
	if (!table_loader) {
		printf("error: no memory for table-loader\n");
		return addr;
	}

	qemu_fwcfg_read_entry(be16_to_cpu(file->cfg.select),
			      size, table_loader);

	for (i = 0; i < (size / sizeof(*entry)); i++) {
		entry = table_loader + i;
		switch (le32_to_cpu(entry->command)) {
		case BIOS_LINKER_LOADER_COMMAND_ALLOCATE:
			ret = bios_linker_allocate(entry, &addr);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_POINTER:
			ret = bios_linker_add_pointer(entry);
			if (ret)
				goto out;
			break;
		case BIOS_LINKER_LOADER_COMMAND_ADD_CHECKSUM:
			ret = bios_linker_add_checksum(entry);
			if (ret)
				goto out;
			break;
		default:
			break;
		}
	}

out:
	if (ret) {
		struct fw_cfg_file_iter iter;
		for (file = qemu_fwcfg_file_iter_init(&iter);
		     !qemu_fwcfg_file_iter_end(&iter);
		     file = qemu_fwcfg_file_iter_next(&iter)) {
			if (file->addr) {
				free((void *)file->addr);
				file->addr = 0;
			}
		}
	}

	free(table_loader);
	return addr;
}

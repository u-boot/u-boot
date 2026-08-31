// SPDX-License-Identifier: GPL-2.0+
/*
 * Common code for EFI commands
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <efi.h>
#include <efi_api.h>
#include <mapmem.h>
#include <u-boot/uuid.h>

/* Width of an EFI physical address in hexadecimal digits */
#define EFI_PHYS_ADDR_WIDTH (int)(sizeof(efi_physical_addr_t) * 2)

/* Width of the memory type column, sized for the longest type name */
#define EFI_MEM_TYPE_WIDTH 23

/*
 * The type names of the UEFI specification without the leading 'Efi' and
 * the trailing 'Type'
 */
static const char *const efi_mem_type_string[] = {
	[EFI_RESERVED_MEMORY_TYPE] = "ReservedMemory",
	[EFI_LOADER_CODE] = "LoaderCode",
	[EFI_LOADER_DATA] = "LoaderData",
	[EFI_BOOT_SERVICES_CODE] = "BootServicesCode",
	[EFI_BOOT_SERVICES_DATA] = "BootServicesData",
	[EFI_RUNTIME_SERVICES_CODE] = "RuntimeServicesCode",
	[EFI_RUNTIME_SERVICES_DATA] = "RuntimeServicesData",
	[EFI_CONVENTIONAL_MEMORY] = "ConventionalMemory",
	[EFI_UNUSABLE_MEMORY] = "UnusableMemory",
	[EFI_ACPI_RECLAIM_MEMORY] = "ACPIReclaimMemory",
	[EFI_ACPI_MEMORY_NVS] = "ACPIMemoryNVS",
	[EFI_MMAP_IO] = "MemoryMappedIO",
	[EFI_MMAP_IO_PORT] = "MemoryMappedIOPortSpace",
	[EFI_PAL_CODE] = "PalCode",
	[EFI_PERSISTENT_MEMORY_TYPE] = "PersistentMemory",
	[EFI_UNACCEPTED_MEMORY_TYPE] = "UnacceptedMemory",
};

static const struct efi_mem_attrs {
	const u64 bit;
	const char *text;
} efi_mem_attrs[] = {
	{EFI_MEMORY_UC, "UC"},
	{EFI_MEMORY_WC, "WC"},
	{EFI_MEMORY_WT, "WT"},
	{EFI_MEMORY_WB, "WB"},
	{EFI_MEMORY_UCE, "UCE"},
	{EFI_MEMORY_WP, "WP"},
	{EFI_MEMORY_RP, "RP"},
	{EFI_MEMORY_XP, "XP"},
	{EFI_MEMORY_NV, "NV"},
	{EFI_MEMORY_MORE_RELIABLE, "REL"},
	{EFI_MEMORY_RO, "RO"},
	{EFI_MEMORY_SP, "SP"},
	{EFI_MEMORY_CPU_CRYPTO, "CRYPT"},
	{EFI_MEMORY_HOT_PLUGGABLE, "HOTPL"},
	{EFI_MEMORY_ISA_VALID, "ISA_VALID"},
	{EFI_MEMORY_RUNTIME, "RT"},
};

void efi_show_tables(struct efi_system_table *systab)
{
	int i;

	for (i = 0; i < systab->nr_tables; i++) {
		struct efi_configuration_table *tab = &systab->tables[i];

		printf("%p  %pUl  %s\n", tab->table, tab->guid.b,
		       uuid_guid_get_str(tab->guid.b) ?: "(unknown)");
	}
}

/**
 * efi_mem_type_name() - get the name of an EFI memory type
 *
 * @type: memory type (enum efi_memory_type)
 * Return: name of the memory type, or NULL if @type is unknown
 */
static const char *efi_mem_type_name(u32 type)
{
	if (type >= ARRAY_SIZE(efi_mem_type_string))
		return NULL;

	return efi_mem_type_string[type];
}

/**
 * efi_print_mem_attrs() - print the names of set EFI memory attributes
 *
 * Prints the set attribute bits as a '|'-separated list of mnemonics,
 * e.g. ' UC|WB|RT', preceded by a space. When EFI_MEMORY_ISA_VALID is
 * set, the EFI_MEMORY_ISA_MASK field is printed as ISA=<value>. Bits
 * that have no mnemonic are printed as a hexadecimal value so that
 * invalid or not yet known attributes are never dropped silently.
 *
 * @attributes: memory attributes (EFI_MEMORY_...)
 */
static void efi_print_mem_attrs(u64 attributes)
{
	u64 unknown = attributes;
	int sep, i;

	for (sep = 0, i = 0; i < ARRAY_SIZE(efi_mem_attrs); i++)
		if (attributes & efi_mem_attrs[i].bit) {
			if (sep) {
				putc('|');
			} else {
				putc(' ');
				sep = 1;
			}
			puts(efi_mem_attrs[i].text);
			unknown &= ~efi_mem_attrs[i].bit;
		}

	if (attributes & EFI_MEMORY_ISA_VALID) {
		printf("%sISA=0x%llx", sep ? "|" : " ",
		       (attributes & EFI_MEMORY_ISA_MASK) >>
		       EFI_MEMORY_ISA_SHIFT);
		sep = 1;
		unknown &= ~EFI_MEMORY_ISA_MASK;
	}

	if (unknown)
		printf("%s0x%llx", sep ? "|" : " ", unknown);
}

void efi_show_memmap(struct efi_mem_desc *map, efi_uintn_t map_size,
		     efi_uintn_t desc_size)
{
	struct efi_mem_desc *end = (void *)map + map_size;
	static const char sep[] = "========================";
	const char *type;

	printf("%-*s %-*s %-*s Attributes\n",
	       EFI_MEM_TYPE_WIDTH, "Type",
	       EFI_PHYS_ADDR_WIDTH, "Start", EFI_PHYS_ADDR_WIDTH, "End");
	printf("%.*s %.*s %.*s ==========\n",
	       EFI_MEM_TYPE_WIDTH, sep,
	       EFI_PHYS_ADDR_WIDTH, sep, EFI_PHYS_ADDR_WIDTH, sep);

	for (; map < end; map = efi_get_next_mem_desc(map, desc_size)) {
		type = efi_mem_type_name(map->type) ?: "(unknown)";

		printf("%-*s %.*llx-%.*llx", EFI_MEM_TYPE_WIDTH, type,
		       EFI_PHYS_ADDR_WIDTH,
		       (u64)map_to_sysmem((void *)(uintptr_t)
					  map->physical_start),
		       EFI_PHYS_ADDR_WIDTH,
		       (u64)map_to_sysmem((void *)(uintptr_t)
					  (map->physical_start +
					   map->num_pages * EFI_PAGE_SIZE)));

		efi_print_mem_attrs(map->attribute);
		putc('\n');
	}
}

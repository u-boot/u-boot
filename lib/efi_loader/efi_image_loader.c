/*
 *  EFI image loader
 *
 *  based partly on wine code
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>
#include <pe.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

const efi_guid_t efi_guid_device_path = DEVICE_PATH_GUID;
const efi_guid_t efi_guid_loaded_image = LOADED_IMAGE_GUID;

efi_status_t EFIAPI efi_return_handle(void *handle, efi_guid_t *protocol,
			void **protocol_interface, void *agent_handle,
			void *controller_handle, uint32_t attributes)
{
	*protocol_interface = handle;
	return EFI_SUCCESS;
}

static void efi_loader_relocate(const IMAGE_BASE_RELOCATION *rel,
			unsigned long rel_size, void *efi_reloc)
{
	const IMAGE_BASE_RELOCATION *end;
	int i;

	end = (const IMAGE_BASE_RELOCATION *)((const char *)rel + rel_size);
	while (rel < end - 1 && rel->SizeOfBlock) {
		const uint16_t *relocs = (const uint16_t *)(rel + 1);
		i = (rel->SizeOfBlock - sizeof(*rel)) / sizeof(uint16_t);
		while (i--) {
			uint32_t offset = (uint32_t)(*relocs & 0xfff) +
					  rel->VirtualAddress;
			int type = *relocs >> EFI_PAGE_SHIFT;
			unsigned long delta = (unsigned long)efi_reloc;
			uint64_t *x64 = efi_reloc + offset;
			uint32_t *x32 = efi_reloc + offset;
			uint16_t *x16 = efi_reloc + offset;

			switch (type) {
			case IMAGE_REL_BASED_ABSOLUTE:
				break;
			case IMAGE_REL_BASED_HIGH:
				*x16 += ((uint32_t)delta) >> 16;
				break;
			case IMAGE_REL_BASED_LOW:
				*x16 += (uint16_t)delta;
				break;
			case IMAGE_REL_BASED_HIGHLOW:
				*x32 += (uint32_t)delta;
				break;
			case IMAGE_REL_BASED_DIR64:
				*x64 += (uint64_t)delta;
				break;
			default:
				printf("Unknown Relocation off %x type %x\n",
				       offset, type);
			}
			relocs++;
		}
		rel = (const IMAGE_BASE_RELOCATION *)relocs;
	}
}

void __weak invalidate_icache_all(void)
{
	/* If the system doesn't support icache_all flush, cross our fingers */
}

/*
 * This function loads all sections from a PE binary into a newly reserved
 * piece of memory. On successful load it then returns the entry point for
 * the binary. Otherwise NULL.
 */
void *efi_load_pe(void *efi, struct efi_loaded_image *loaded_image_info)
{
	IMAGE_NT_HEADERS32 *nt;
	IMAGE_DOS_HEADER *dos;
	IMAGE_SECTION_HEADER *sections;
	int num_sections;
	void *efi_reloc;
	int i;
	const IMAGE_BASE_RELOCATION *rel;
	unsigned long rel_size;
	int rel_idx = IMAGE_DIRECTORY_ENTRY_BASERELOC;
	void *entry;
	uint64_t image_size;
	unsigned long virt_size = 0;
	bool can_run_nt64 = true;
	bool can_run_nt32 = true;

#if defined(CONFIG_ARM64)
	can_run_nt32 = false;
#elif defined(CONFIG_ARM)
	can_run_nt64 = false;
#endif

	dos = efi;
	if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
		printf("%s: Invalid DOS Signature\n", __func__);
		return NULL;
	}

	nt = (void *) ((char *)efi + dos->e_lfanew);
	if (nt->Signature != IMAGE_NT_SIGNATURE) {
		printf("%s: Invalid NT Signature\n", __func__);
		return NULL;
	}

	/* Calculate upper virtual address boundary */
	num_sections = nt->FileHeader.NumberOfSections;
	sections = (void *)&nt->OptionalHeader +
			    nt->FileHeader.SizeOfOptionalHeader;

	for (i = num_sections - 1; i >= 0; i--) {
		IMAGE_SECTION_HEADER *sec = &sections[i];
		virt_size = max_t(unsigned long, virt_size,
				  sec->VirtualAddress + sec->Misc.VirtualSize);
	}

	/* Read 32/64bit specific header bits */
	if (can_run_nt64 &&
	    (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)) {
		IMAGE_NT_HEADERS64 *nt64 = (void *)nt;
		IMAGE_OPTIONAL_HEADER64 *opt = &nt64->OptionalHeader;
		image_size = opt->SizeOfImage;
		efi_reloc = efi_alloc(virt_size, EFI_LOADER_DATA);
		if (!efi_reloc) {
			printf("%s: Could not allocate %ld bytes\n",
				__func__, virt_size);
			return NULL;
		}
		entry = efi_reloc + opt->AddressOfEntryPoint;
		rel_size = opt->DataDirectory[rel_idx].Size;
		rel = efi_reloc + opt->DataDirectory[rel_idx].VirtualAddress;
	} else if (can_run_nt32 &&
		   (nt->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)) {
		IMAGE_OPTIONAL_HEADER32 *opt = &nt->OptionalHeader;
		image_size = opt->SizeOfImage;
		efi_reloc = efi_alloc(virt_size, EFI_LOADER_DATA);
		if (!efi_reloc) {
			printf("%s: Could not allocate %ld bytes\n",
				__func__, virt_size);
			return NULL;
		}
		entry = efi_reloc + opt->AddressOfEntryPoint;
		rel_size = opt->DataDirectory[rel_idx].Size;
		rel = efi_reloc + opt->DataDirectory[rel_idx].VirtualAddress;
	} else {
		printf("%s: Invalid optional header magic %x\n", __func__,
		       nt->OptionalHeader.Magic);
		return NULL;
	}

	/* Load sections into RAM */
	for (i = num_sections - 1; i >= 0; i--) {
		IMAGE_SECTION_HEADER *sec = &sections[i];
		memset(efi_reloc + sec->VirtualAddress, 0,
		       sec->Misc.VirtualSize);
		memcpy(efi_reloc + sec->VirtualAddress,
		       efi + sec->PointerToRawData,
		       sec->SizeOfRawData);
	}

	/* Run through relocations */
	efi_loader_relocate(rel, rel_size, efi_reloc);

	/* Flush cache */
	flush_cache((ulong)efi_reloc, virt_size);
	invalidate_icache_all();

	/* Populate the loaded image interface bits */
	loaded_image_info->image_base = efi;
	loaded_image_info->image_size = image_size;

	return entry;
}

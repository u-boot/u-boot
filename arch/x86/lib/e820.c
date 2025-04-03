// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY	LOGC_ARCH

#include <efi_loader.h>
#include <lmb.h>
#include <log.h>
#include <asm/e820.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const char *const e820_type_name[E820_COUNT] = {
	[E820_RAM] = "RAM",
	[E820_RESERVED] = "Reserved",
	[E820_ACPI] = "ACPI",
	[E820_NVS] = "ACPI NVS",
	[E820_UNUSABLE] = "Unusable",
};

void e820_dump(struct e820_entry *entries, uint count)
{
	int i;

	printf("%12s  %10s  %s\n", "Addr", "Size", "Type");
	for (i = 0; i < count; i++) {
		struct e820_entry *entry = &entries[i];

		printf("%12llx  %10llx  %s\n", entry->addr, entry->size,
		       entry->type < E820_COUNT ?
		       e820_type_name[entry->type] :
		       simple_itoa(entry->type));
	}
}

/*
 * Install a default e820 table with 4 entries as follows:
 *
 *	0x000000-0x0a0000	Useable RAM
 *	0x0a0000-0x100000	Reserved for ISA
 *	0x100000-gd->ram_size	Useable RAM
 *	CONFIG_PCIE_ECAM_BASE	PCIe ECAM
 */
__weak unsigned int install_e820_map(unsigned int max_entries,
				     struct e820_entry *entries)
{
	entries[0].addr = 0;
	entries[0].size = ISA_START_ADDRESS;
	entries[0].type = E820_RAM;
	entries[1].addr = ISA_START_ADDRESS;
	entries[1].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[1].type = E820_RESERVED;
	entries[2].addr = ISA_END_ADDRESS;
	entries[2].size = gd->ram_size - ISA_END_ADDRESS;
	entries[2].type = E820_RAM;
	entries[3].addr = CONFIG_PCIE_ECAM_BASE;
	entries[3].size = CONFIG_PCIE_ECAM_SIZE;
	entries[3].type = E820_RESERVED;

	return 4;
}

void e820_init(struct e820_ctx *ctx, struct e820_entry *entries,
	       int max_entries)
{
	memset(ctx, '\0', sizeof(*ctx));
	ctx->entries = entries;
	ctx->max_entries = max_entries;
}

void e820_add(struct e820_ctx *ctx, enum e820_type type, u64 addr, u64 size)
{
	struct e820_entry *entry = &ctx->entries[ctx->count++];

	if (ctx->count <= ctx->max_entries) {
		entry->addr = addr;
		entry->size = size;
		entry->type = type;
	}
	ctx->addr = addr + size;
}

void e820_next(struct e820_ctx *ctx, enum e820_type type, u64 size)
{
	e820_add(ctx, type, ctx->addr, size);
}

void e820_to_addr(struct e820_ctx *ctx, enum e820_type type, u64 addr)
{
	e820_next(ctx, type, addr - ctx->addr);
}

int e820_finish(struct e820_ctx *ctx)
{
	if (ctx->count > ctx->max_entries) {
		printf("e820 has %d entries but room for only %d\n", ctx->count,
		       ctx->max_entries);
		panic("e820 table too large");
	}
	log_debug("e820 map installed, n=%d\n", ctx->count);
	if (_DEBUG)
		e820_dump(ctx->entries, ctx->count);

	return ctx->count;
}

#if CONFIG_IS_ENABLED(EFI_LOADER)
void efi_add_known_memory(void)
{
	struct e820_entry e820[E820MAX];
	unsigned int i, num;
	u64 start;
	int type;

	num = install_e820_map(ARRAY_SIZE(e820), e820);

	for (i = 0; i < num; ++i) {
		start = e820[i].addr;

		switch (e820[i].type) {
		case E820_RAM:
			type = EFI_CONVENTIONAL_MEMORY;
			break;
		case E820_RESERVED:
			type = EFI_RESERVED_MEMORY_TYPE;
			break;
		case E820_ACPI:
			type = EFI_ACPI_RECLAIM_MEMORY;
			break;
		case E820_NVS:
			type = EFI_ACPI_MEMORY_NVS;
			break;
		case E820_UNUSABLE:
		default:
			type = EFI_UNUSABLE_MEMORY;
			break;
		}

		if (type != EFI_CONVENTIONAL_MEMORY)
			efi_add_memory_map(start, e820[i].size, type);
	}
}
#endif /* CONFIG_IS_ENABLED(EFI_LOADER) */

#if CONFIG_IS_ENABLED(LMB_ARCH_MEM_MAP)
void lmb_arch_add_memory(void)
{
	struct e820_entry e820[E820MAX];
	unsigned int i, num;
	u64 ram_top;

	num = install_e820_map(ARRAY_SIZE(e820), e820);

	ram_top = (u64)gd->ram_top & ~EFI_PAGE_MASK;
	if (!ram_top)
		ram_top = 0x100000000ULL;

	for (i = 0; i < num; ++i) {
		if (e820[i].type == E820_RAM) {
			u64 start, size, rgn_top;

			start = e820[i].addr;
			size = e820[i].size;
			rgn_top = start + size;

			if (start > ram_top)
				continue;

			if (rgn_top > ram_top)
				size -= rgn_top - ram_top;

			lmb_add(start, size);
		}
	}
}
#endif /* CONFIG_IS_ENABLED(LMB_ARCH_MEM_MAP) */

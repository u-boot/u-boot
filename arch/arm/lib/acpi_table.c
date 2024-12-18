// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2024 9elements GmbH
 */

#define LOG_CATEGORY LOGC_ACPI

#include <bloblist.h>
#include <cpu_func.h>
#include <efi_loader.h>
#include <malloc.h>
#include <string.h>
#include <tables_csum.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_table.h>
#include <asm-generic/io.h>
#include <dm/acpi.h>
#include <dm/uclass.h>
#include <linux/log2.h>
#include <linux/sizes.h>

/* defined in assembly file */
/**
 * acpi_pp_code_size - Spinloop code size *
 */
extern u16 acpi_pp_code_size;

/**
 * acpi_pp_tables - Start of ACPI PP tables.
 */
extern ulong acpi_pp_tables;

/**
 * acpi_pp_etables - End of ACPI PP tables.
 */
extern ulong acpi_pp_etables;

/**
 * acpi_pp_code_start() - Spinloop code
 *
 * Architectural spinloop code to be installed in each parking protocol
 * page. The spinloop code must be less than 2048 bytes.
 *
 * The spinloop code will be entered after calling
 * acpi_parking_protocol_install().
 *
 */
void acpi_pp_code_start(void);

void acpi_write_madt_gicc(struct acpi_madt_gicc *gicc, uint cpu_num,
			  uint perf_gsiv, ulong phys_base, ulong gicv,
			  ulong gich, uint vgic_maint_irq, u64 gicr_base,
			  ulong mpidr, uint efficiency)
{
	memset(gicc, '\0', sizeof(struct acpi_madt_gicc));
	gicc->type = ACPI_APIC_GICC;
	gicc->length = sizeof(struct acpi_madt_gicc);
	gicc->cpu_if_num = cpu_num;
	gicc->processor_id = cpu_num;
	gicc->flags = ACPI_MADTF_ENABLED;
	gicc->perf_gsiv = perf_gsiv;
	gicc->phys_base = phys_base;
	gicc->gicv = gicv;
	gicc->gich = gich;
	gicc->vgic_maint_irq = vgic_maint_irq;
	gicc->gicr_base = gicr_base;
	gicc->mpidr = mpidr;
	gicc->efficiency = efficiency;
}

void acpi_write_madt_gicd(struct acpi_madt_gicd *gicd, uint gic_id,
			  ulong phys_base, uint gic_version)
{
	memset(gicd, '\0', sizeof(struct acpi_madt_gicd));
	gicd->type = ACPI_APIC_GICD;
	gicd->length = sizeof(struct acpi_madt_gicd);
	gicd->gic_id = gic_id;
	gicd->phys_base = phys_base;
	gicd->gic_version = gic_version;
}

void acpi_write_madt_gicr(struct acpi_madt_gicr *gicr,
			  u64 discovery_range_base_address,
			  u32 discovery_range_length)
{
	memset(gicr, '\0', sizeof(struct acpi_madt_gicr));
	gicr->type = ACPI_APIC_GICR;
	gicr->length = sizeof(struct acpi_madt_gicr);
	gicr->discovery_range_base_address = discovery_range_base_address;
	gicr->discovery_range_length = discovery_range_length;
}

void acpi_write_madt_its(struct acpi_madt_its *its,
			 u32 its_id,
			 u64 physical_base_address)
{
	memset(its, '\0', sizeof(struct acpi_madt_its));
	its->type = ACPI_APIC_ITS;
	its->length = sizeof(struct acpi_madt_its);
	its->gic_its_id = its_id;
	its->physical_base_address = physical_base_address;
}

int acpi_pptt_add_proc(struct acpi_ctx *ctx, const u32 flags, const u32 parent,
		       const u32 proc_id, const u32 num_resources,
		       const u32 *resource_list)
{
	struct acpi_pptt_proc *proc = ctx->current;
	int offset;

	offset = ctx->current - ctx->tab_start;
	proc->hdr.type = ACPI_PPTT_TYPE_PROC;
	proc->flags = flags;
	proc->parent = parent;
	proc->proc_id = proc_id;
	proc->num_resources = num_resources;
	proc->hdr.length = sizeof(struct acpi_pptt_proc) +
		sizeof(u32) * num_resources;

	if (resource_list)
		memcpy(proc + 1, resource_list, sizeof(u32) * num_resources);

	acpi_inc(ctx, proc->hdr.length);

	return offset;
}

int acpi_pptt_add_cache(struct acpi_ctx *ctx, const u32 flags,
			const u32 next_cache_level, const u32 size,
			const u32 sets, const u8 assoc, const u8 attributes,
			const u16 line_size)
{
	struct acpi_pptt_cache *cache = ctx->current;
	int offset;

	offset = ctx->current - ctx->tab_start;
	cache->hdr.type = ACPI_PPTT_TYPE_CACHE;
	cache->hdr.length = sizeof(struct acpi_pptt_cache);
	cache->flags = flags;
	cache->next_cache_level = next_cache_level;
	cache->size = size;
	cache->sets = sets;
	cache->assoc = assoc;
	cache->attributes = attributes;
	cache->line_size = line_size;
	acpi_inc(ctx, cache->hdr.length);

	return offset;
}

void *acpi_fill_madt(struct acpi_madt *madt, struct acpi_ctx *ctx)
{
	uclass_probe_all(UCLASS_CPU);
	uclass_probe_all(UCLASS_IRQ);

	/* All SoCs must use the driver model */
	acpi_fill_madt_subtbl(ctx);

	return ctx->current;
}

/**
 * acpi_write_pp_setup_one_page() - Fill out one page used by the PP
 *
 * Fill out the struct acpi_pp_page to contain the spin-loop
 * code and the mailbox area. After this function the page is ready for
 * the secondary core's to enter the spin-loop code.
 *
 * @page:                 Pointer to current parking protocol page
 * @gicc:                 Pointer to corresponding GICC sub-table
 */
static void acpi_write_pp_setup_one_page(struct acpi_pp_page *page,
					 struct acpi_madt_gicc *gicc)
{
	void *reloc;

	/* Update GICC. Mark parking protocol as available. */
	gicc->parking_proto = ACPI_PP_VERSION;
	gicc->parked_addr = virt_to_phys(page);

	/* Prepare parking protocol page */
	memset(page, '\0', sizeof(struct acpi_pp_page));

	/* Init mailbox. Set MPIDR so core's will find their page. */
	page->cpu_id = gicc->mpidr;
	page->jumping_address = ACPI_PP_JMP_ADR_INVALID;

	/* Relocate spinning code */
	reloc = &page->spinning_code[0];

	log_debug("Relocating spin table from %lx to %lx (size %x)\n",
		  (ulong)&acpi_pp_code_start, (ulong)reloc, acpi_pp_code_size);
	memcpy(reloc, &acpi_pp_code_start, acpi_pp_code_size);

	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		flush_dcache_range((unsigned long)page,
				   (unsigned long)(page + 1));
}

void acpi_write_park(struct acpi_madt *madt)
{
	struct acpi_pp_page *start, *page;
	struct acpi_madt_gicc *gicc;
	int ret, i, ncpus = 0;

	/*
	 * According to the "Multi-processor Startup for ARM Platforms":
	 * - Every CPU as specified by MADT GICC has it's own 4K page
	 * - Every page is divided into two sections: OS and FW reserved
	 * - Memory occupied by "Parking Protocol" must be marked 'Reserved'
	 * - Spinloop code should reside in FW reserved 2048 bytes
	 * - Spinloop code will check the mailbox in OS reserved area
	 */

	if (acpi_pp_code_size > sizeof(page->spinning_code)) {
		log_err("Spinning code too big to fit: %d\n",
			acpi_pp_code_size);
		return;
	}

	/* Count all MADT GICCs including BSP */
	for (i = sizeof(struct acpi_madt); i < madt->header.length;
	     i += gicc->length) {
		gicc = (struct acpi_madt_gicc *)((void *)madt + i);
		if (gicc->type != ACPI_APIC_GICC)
			continue;
		ncpus++;
	}
	log_debug("Found %#x GICCs in MADT\n", ncpus);

	/* Allocate pages linearly due to assembly code requirements */
	start = bloblist_add(BLOBLISTT_ACPI_PP, ACPI_PP_PAGE_SIZE * ncpus,
			     ilog2(SZ_4K));
	if (!start) {
		log_err("Failed to allocate memory for ACPI-parking-protocol pages\n");
		return;
	}
	log_debug("Allocated parking protocol at %p\n", start);
	page = start;

	if (IS_ENABLED(CONFIG_EFI_LOADER)) {
		/* Default mapping is 'BOOT CODE'. Mark as reserved instead. */
		ret = efi_add_memory_map((u64)(uintptr_t)start,
					 ncpus * ACPI_PP_PAGE_SIZE,
					 EFI_RESERVED_MEMORY_TYPE);

		if (ret)
			log_err("Reserved memory mapping failed addr %p size %x\n",
				start, ncpus * ACPI_PP_PAGE_SIZE);
	}

	/* Prepare the parking protocol pages */
	for (i = sizeof(struct acpi_madt); i < madt->header.length;
	     i += gicc->length) {
		gicc = (struct acpi_madt_gicc *)((void *)madt + i);
		if (gicc->type != ACPI_APIC_GICC)
			continue;

		acpi_write_pp_setup_one_page(page++, gicc);
	}

	acpi_pp_etables = virt_to_phys(start) +
			  ACPI_PP_PAGE_SIZE * ncpus;
	acpi_pp_tables = virt_to_phys(start);

	/* Make sure other cores see written value in memory */
	if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
		flush_dcache_all();

	/* Send an event to wake up the secondary CPU. */
	asm("dsb	ishst\n"
	    "sev");
}

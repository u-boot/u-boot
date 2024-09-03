// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2024 9elements GmbH
 */

#define LOG_CATEGORY LOGC_ACPI

#include <string.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <dm/uclass.h>

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

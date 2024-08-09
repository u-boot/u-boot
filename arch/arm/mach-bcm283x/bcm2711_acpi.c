// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2024 9elements GmbH
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <acpi/acpi_table.h>
#include <asm/acpi_table.h>
#include <asm/armv8/sec_firmware.h>
#include <asm/arch/acpi/bcm2711.h>
#include <tables_csum.h>
#include <string.h>

void acpi_fill_fadt(struct acpi_fadt *fadt)
{
	fadt->flags = ACPI_FADT_HW_REDUCED_ACPI | ACPI_FADT_LOW_PWR_IDLE_S0;

	if (CONFIG_IS_ENABLED(SEC_FIRMWARE_ARMV8_PSCI) &&
	    sec_firmware_support_psci_version() != PSCI_INVALID_VER)
		fadt->arm_boot_arch = ACPI_ARM_PSCI_COMPLIANT;
}

void *acpi_fill_madt(struct acpi_madt *madt, void *current)
{
	struct acpi_madt_gicc *gicc;
	struct acpi_madt_gicd *gicd;

	madt->lapic_addr = 0;
	madt->flags = 0;

	gicc = current;
	for (int i = 0; i < 4; i++) {
		acpi_write_madt_gicc(gicc++, i, 0x30 + i, BCM2711_GIC400_BASE_ADDRESS + 0x2000,
				     BCM2711_GIC400_BASE_ADDRESS + 0x6000,
				     BCM2711_GIC400_BASE_ADDRESS + 0x4000,
				     0x19, i, 1);
	}

	gicd = (struct acpi_madt_gicd *)gicc;
	acpi_write_madt_gicd(gicd++, 0, BCM2711_GIC400_BASE_ADDRESS + 0x1000, 2);
	return gicd;
}

static u32 *add_proc(struct acpi_ctx *ctx, int flags, int parent, int proc_id,
		     int num_resources)
{
	struct acpi_pptt_proc *proc = ctx->current;
	u32 *resource_list;

	proc->hdr.type = ACPI_PPTT_TYPE_PROC;
	proc->flags = flags;
	proc->parent = parent;
	proc->proc_id = proc_id;
	proc->num_resources = num_resources;
	proc->hdr.length = sizeof(struct acpi_pptt_proc) +
		sizeof(u32) * num_resources;
	resource_list = ctx->current + sizeof(struct acpi_pptt_proc);
	acpi_inc(ctx, proc->hdr.length);

	return resource_list;
}

static int add_cache(struct acpi_ctx *ctx, int flags, int size, int sets,
		     int assoc, int attributes, int line_size)
{
	struct acpi_pptt_cache *cache = ctx->current;
	int ofs;

	ofs = ctx->current - ctx->tab_start;
	cache->hdr.type = ACPI_PPTT_TYPE_CACHE;
	cache->hdr.length = sizeof(struct acpi_pptt_cache);
	cache->flags = flags;
	cache->next_cache_level = 0;
	cache->size = size;
	cache->sets = sets;
	cache->assoc = assoc;
	cache->attributes = attributes;
	cache->line_size = line_size;
	acpi_inc(ctx, cache->hdr.length);

	return ofs;
}

static int acpi_write_pptt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	int proc_ofs;
	u32 *proc_ptr;
	int ofs, ofs0, ofs1, i;

	header = ctx->current;
	ctx->tab_start = ctx->current;

	memset(header, '\0', sizeof(struct acpi_table_header));

	acpi_fill_header(header, "PPTT");
	header->revision = acpi_get_table_revision(ACPITAB_PPTT);
	acpi_inc(ctx, sizeof(*header));

	proc_ofs = ctx->current - ctx->tab_start;
	proc_ptr = add_proc(ctx, ACPI_PPTT_PHYSICAL_PACKAGE |
			    ACPI_PPTT_CHILDREN_IDENTICAL, 0, 0, 1);

	ofs = add_cache(ctx, ACPI_PPTT_ALL_VALID, 0x100000, 0x400, 0x10,
			ACPI_PPTT_WRITE_ALLOC |
			(ACPI_PPTT_CACHE_TYPE_UNIFIED <<
			 ACPI_PPTT_CACHE_TYPE_SHIFT), 0x40);
	*proc_ptr = ofs;

	for (i = 0; i < 4; i++) {
		proc_ptr = add_proc(ctx, ACPI_PPTT_CHILDREN_IDENTICAL |
				    ACPI_PPTT_NODE_IS_LEAF | ACPI_PPTT_PROC_ID_VALID,
				    proc_ofs, i, 2);

		ofs0 = add_cache(ctx, ACPI_PPTT_ALL_VALID, 0x8000, 0x100, 2,
				 ACPI_PPTT_WRITE_ALLOC, 0x40);

		ofs1 = add_cache(ctx, ACPI_PPTT_ALL_BUT_WRITE_POL, 0xc000, 0x100, 3,
				 ACPI_PPTT_CACHE_TYPE_INSTR <<
				 ACPI_PPTT_CACHE_TYPE_SHIFT, 0x40);
		proc_ptr[0] = ofs0;
		proc_ptr[1] = ofs1;
	}

	header->length = ctx->current - ctx->tab_start;
	header->checksum = table_compute_checksum(header, header->length);

	acpi_inc(ctx, header->length);
	acpi_add_table(ctx, header);

	return 0;
};

ACPI_WRITER(5pptt, "PPTT", acpi_write_pptt, 0);

static int rpi_write_gtdt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_gtdt *gtdt;

	gtdt = ctx->current;
	header = &gtdt->header;

	memset(gtdt, '\0', sizeof(struct acpi_gtdt));

	acpi_fill_header(header, "GTDT");
	header->length = sizeof(struct acpi_gtdt);
	header->revision = acpi_get_table_revision(ACPITAB_GTDT);

	gtdt->cnt_ctrl_base = BCM2711_ARM_LOCAL_BASE_ADDRESS + 0x1c;
	gtdt->sec_el1_gsiv = 29;
	gtdt->sec_el1_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->el1_gsiv = 30;
	gtdt->el1_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->virt_el1_gsiv = 27;
	gtdt->virt_el1_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->el2_gsiv = 26;
	gtdt->el2_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->cnt_read_base = 0xffffffffffffffff;

	header->checksum = table_compute_checksum(header, header->length);

	acpi_add_table(ctx, gtdt);

	acpi_inc(ctx, sizeof(struct acpi_gtdt));

	return 0;
};

ACPI_WRITER(5gtdt, "GTDT", rpi_write_gtdt, 0);
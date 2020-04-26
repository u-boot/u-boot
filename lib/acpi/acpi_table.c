// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic code used to generate ACPI tables
 *
 * Copyright 2019 Google LLC
 */

#include <common.h>
#include <dm.h>
#include <cpu.h>
#include <mapmem.h>
#include <tables_csum.h>
#include <version.h>
#include <acpi/acpi_table.h>
#include <dm/acpi.h>

int acpi_create_dmar(struct acpi_dmar *dmar, enum dmar_flags flags)
{
	struct acpi_table_header *header = &dmar->header;
	struct cpu_info info;
	struct udevice *cpu;
	int ret;

	ret = uclass_first_device(UCLASS_CPU, &cpu);
	if (ret)
		return log_msg_ret("cpu", ret);
	ret = cpu_get_info(cpu, &info);
	if (ret)
		return log_msg_ret("info", ret);
	memset((void *)dmar, 0, sizeof(struct acpi_dmar));

	/* Fill out header fields. */
	acpi_fill_header(&dmar->header, "DMAR");
	header->length = sizeof(struct acpi_dmar);
	header->revision = acpi_get_table_revision(ACPITAB_DMAR);

	dmar->host_address_width = info.address_width - 1;
	dmar->flags = flags;

	return 0;
}

int acpi_get_table_revision(enum acpi_tables table)
{
	switch (table) {
	case ACPITAB_FADT:
		return ACPI_FADT_REV_ACPI_3_0;
	case ACPITAB_MADT:
		return ACPI_MADT_REV_ACPI_3_0;
	case ACPITAB_MCFG:
		return ACPI_MCFG_REV_ACPI_3_0;
	case ACPITAB_TCPA:
		/* This version and the rest are open-coded */
		return 2;
	case ACPITAB_TPM2:
		return 4;
	case ACPITAB_SSDT: /* ACPI 3.0 upto 6.3: 2 */
		return 2;
	case ACPITAB_SRAT: /* ACPI 2.0: 1, ACPI 3.0: 2, ACPI 4.0 to 6.3: 3 */
		return 1; /* TODO Should probably be upgraded to 2 */
	case ACPITAB_DMAR:
		return 1;
	case ACPITAB_SLIT: /* ACPI 2.0 upto 6.3: 1 */
		return 1;
	case ACPITAB_SPMI: /* IMPI 2.0 */
		return 5;
	case ACPITAB_HPET: /* Currently 1. Table added in ACPI 2.0 */
		return 1;
	case ACPITAB_VFCT: /* ACPI 2.0/3.0/4.0: 1 */
		return 1;
	case ACPITAB_IVRS:
		return IVRS_FORMAT_FIXED;
	case ACPITAB_DBG2:
		return 0;
	case ACPITAB_FACS: /* ACPI 2.0/3.0: 1, ACPI 4.0 to 6.3: 2 */
		return 1;
	case ACPITAB_RSDT: /* ACPI 1.0 upto 6.3: 1 */
		return 1;
	case ACPITAB_XSDT: /* ACPI 2.0 upto 6.3: 1 */
		return 1;
	case ACPITAB_RSDP: /* ACPI 2.0 upto 6.3: 2 */
		return 2;
	case ACPITAB_HEST:
		return 1;
	case ACPITAB_NHLT:
		return 5;
	case ACPITAB_BERT:
		return 1;
	case ACPITAB_SPCR:
		return 2;
	default:
		return -EINVAL;
	}
}

void acpi_fill_header(struct acpi_table_header *header, char *signature)
{
	memcpy(header->signature, signature, 4);
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	header->oem_revision = U_BOOT_BUILD_DATE;
	memcpy(header->aslc_id, ASLC_ID, 4);
}

void acpi_align(struct acpi_ctx *ctx)
{
	ctx->current = (void *)ALIGN((ulong)ctx->current, 16);
}

void acpi_align64(struct acpi_ctx *ctx)
{
	ctx->current = (void *)ALIGN((ulong)ctx->current, 64);
}

void acpi_inc(struct acpi_ctx *ctx, uint amount)
{
	ctx->current += amount;
}

void acpi_inc_align(struct acpi_ctx *ctx, uint amount)
{
	ctx->current += amount;
	acpi_align(ctx);
}

/**
 * Add an ACPI table to the RSDT (and XSDT) structure, recalculate length
 * and checksum.
 */
int acpi_add_table(struct acpi_ctx *ctx, void *table)
{
	int i, entries_num;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;

	/* The RSDT is mandatory while the XSDT is not */
	rsdt = ctx->rsdt;

	/* This should always be MAX_ACPI_TABLES */
	entries_num = ARRAY_SIZE(rsdt->entry);

	for (i = 0; i < entries_num; i++) {
		if (rsdt->entry[i] == 0)
			break;
	}

	if (i >= entries_num) {
		log_err("ACPI: Error: too many tables\n");
		return -E2BIG;
	}

	/* Add table to the RSDT */
	rsdt->entry[i] = map_to_sysmem(table);

	/* Fix RSDT length or the kernel will assume invalid entries */
	rsdt->header.length = sizeof(struct acpi_table_header) +
				(sizeof(u32) * (i + 1));

	/* Re-calculate checksum */
	rsdt->header.checksum = 0;
	rsdt->header.checksum = table_compute_checksum((u8 *)rsdt,
						       rsdt->header.length);

	/*
	 * And now the same thing for the XSDT. We use the same index as for
	 * now we want the XSDT and RSDT to always be in sync in U-Boot
	 */
	xsdt = map_sysmem(ctx->rsdp->xsdt_address, sizeof(*xsdt));

	/* Add table to the XSDT */
	xsdt->entry[i] = map_to_sysmem(table);

	/* Fix XSDT length */
	xsdt->header.length = sizeof(struct acpi_table_header) +
				(sizeof(u64) * (i + 1));

	/* Re-calculate checksum */
	xsdt->header.checksum = 0;
	xsdt->header.checksum = table_compute_checksum((u8 *)xsdt,
						       xsdt->header.length);

	return 0;
}

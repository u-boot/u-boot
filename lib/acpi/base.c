// SPDX-License-Identifier: GPL-2.0+
/*
 * Write base ACPI tables
 *
 * Copyright 2021 Google LLC
 */

#define LOG_CATEGORY LOGC_ACPI

#include <common.h>
#include <acpi/acpi_table.h>
#include <dm/acpi.h>
#include <mapmem.h>
#include <tables_csum.h>

void acpi_write_rsdp(struct acpi_rsdp *rsdp, struct acpi_rsdt *rsdt,
		     struct acpi_xsdt *xsdt)
{
	memset(rsdp, 0, sizeof(struct acpi_rsdp));

	memcpy(rsdp->signature, RSDP_SIG, 8);
	memcpy(rsdp->oem_id, OEM_ID, 6);

	rsdp->length = sizeof(struct acpi_rsdp);
	rsdp->rsdt_address = map_to_sysmem(rsdt);

	rsdp->xsdt_address = map_to_sysmem(xsdt);
	rsdp->revision = ACPI_RSDP_REV_ACPI_2_0;

	/* Calculate checksums */
	rsdp->checksum = table_compute_checksum(rsdp, 20);
	rsdp->ext_checksum = table_compute_checksum(rsdp,
						    sizeof(struct acpi_rsdp));
}

static void acpi_write_rsdt(struct acpi_rsdt *rsdt)
{
	struct acpi_table_header *header = &rsdt->header;

	/* Fill out header fields */
	acpi_fill_header(header, "RSDT");
	header->length = sizeof(struct acpi_rsdt);
	header->revision = 1;

	/* Entries are filled in later, we come with an empty set */

	/* Fix checksum */
	header->checksum = table_compute_checksum(rsdt,
						  sizeof(struct acpi_rsdt));
}

static void acpi_write_xsdt(struct acpi_xsdt *xsdt)
{
	struct acpi_table_header *header = &xsdt->header;

	/* Fill out header fields */
	acpi_fill_header(header, "XSDT");
	header->length = sizeof(struct acpi_xsdt);
	header->revision = 1;

	/* Entries are filled in later, we come with an empty set */

	/* Fix checksum */
	header->checksum = table_compute_checksum(xsdt,
						  sizeof(struct acpi_xsdt));
}

static int acpi_write_base(struct acpi_ctx *ctx,
			   const struct acpi_writer *entry)
{
	/* We need at least an RSDP and an RSDT Table */
	ctx->rsdp = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_rsdp));
	ctx->rsdt = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_rsdt));
	ctx->xsdt = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_xsdt));

	/* clear all table memory */
	memset(ctx->base, '\0', ctx->current - ctx->base);

	acpi_write_rsdp(ctx->rsdp, ctx->rsdt, ctx->xsdt);
	acpi_write_rsdt(ctx->rsdt);
	acpi_write_xsdt(ctx->xsdt);

	return 0;
}
/*
 * Per ACPI spec, the FACS table address must be aligned to a 64-byte boundary
 * (Windows checks this, but Linux does not).
 *
 * Use the '0' prefix to put this one first
 */
ACPI_WRITER(0base, NULL, acpi_write_base, ACPIWF_ALIGN64);

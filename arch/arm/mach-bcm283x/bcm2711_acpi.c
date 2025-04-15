// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2024 9elements GmbH
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 */

#include <string.h>
#include <tables_csum.h>
#include <acpi/acpi_table.h>
#include <asm/acpi_table.h>
#include <asm/armv8/sec_firmware.h>
#include <asm/arch/acpi/bcm2711.h>
#include <dm/uclass.h>

void acpi_fill_fadt(struct acpi_fadt *fadt)
{
	fadt->flags = ACPI_FADT_HW_REDUCED_ACPI | ACPI_FADT_LOW_PWR_IDLE_S0;

	if (CONFIG_IS_ENABLED(SEC_FIRMWARE_ARMV8_PSCI) &&
	    sec_firmware_support_psci_version() != PSCI_INVALID_VER)
		fadt->arm_boot_arch = ACPI_ARM_PSCI_COMPLIANT;
}

#define L3_ATTRIBUTES (ACPI_PPTT_READ_ALLOC | ACPI_PPTT_WRITE_ALLOC | \
			(ACPI_PPTT_CACHE_TYPE_UNIFIED << \
			 ACPI_PPTT_CACHE_TYPE_SHIFT))
#define L3_SIZE 0x100000
#define L3_SETS 0x400
#define L3_WAYS 0x10

#define L1D_ATTRIBUTES (ACPI_PPTT_READ_ALLOC | ACPI_PPTT_WRITE_ALLOC | \
			(ACPI_PPTT_CACHE_TYPE_DATA << \
			 ACPI_PPTT_CACHE_TYPE_SHIFT))
#define L1D_SIZE 0x8000
#define L1D_SETS 0x100
#define L1D_WAYS 2

#define L1I_ATTRIBUTES (ACPI_PPTT_READ_ALLOC | \
			(ACPI_PPTT_CACHE_TYPE_INSTR << \
			 ACPI_PPTT_CACHE_TYPE_SHIFT))
#define L1I_SIZE 0xc000
#define L1I_SETS 0x100
#define L1I_WAYS 3

static int acpi_write_pptt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	int cluster_offset, l3_offset;
	u32 offsets[2];

	header = ctx->current;
	ctx->tab_start = ctx->current;

	memset(header, '\0', sizeof(struct acpi_table_header));

	acpi_fill_header(header, "PPTT");
	header->revision = acpi_get_table_revision(ACPITAB_PPTT);
	acpi_inc(ctx, sizeof(*header));

	l3_offset = acpi_pptt_add_cache(ctx, ACPI_PPTT_ALL_VALID, 0, L3_SIZE,
					L3_SETS, L3_WAYS, L3_ATTRIBUTES, 64);

	cluster_offset = acpi_pptt_add_proc(ctx, ACPI_PPTT_PHYSICAL_PACKAGE |
					    ACPI_PPTT_CHILDREN_IDENTICAL,
					    0, 0, 1, &l3_offset);

	offsets[0] = acpi_pptt_add_cache(ctx, ACPI_PPTT_ALL_VALID, 0, L1D_SIZE,
					 L1D_SETS, L1D_WAYS, L1D_ATTRIBUTES, 64);

	offsets[1] = acpi_pptt_add_cache(ctx, ACPI_PPTT_ALL_BUT_WRITE_POL, 0,
					 L1I_SIZE, L1I_SETS, L1I_WAYS,
					 L1I_ATTRIBUTES, 64);

	for (int i = 0; i < uclass_id_count(UCLASS_CPU); i++) {
		acpi_pptt_add_proc(ctx, ACPI_PPTT_CHILDREN_IDENTICAL |
				   ACPI_PPTT_NODE_IS_LEAF |
				   ACPI_PPTT_PROC_ID_VALID,
				   cluster_offset, i, 2, offsets);
	}

	header->length = ctx->current - ctx->tab_start;
	acpi_update_checksum(header);

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

	acpi_update_checksum(header);

	acpi_add_table(ctx, gtdt);

	acpi_inc(ctx, sizeof(struct acpi_gtdt));

	return 0;
};

ACPI_WRITER(5gtdt, "GTDT", rpi_write_gtdt, 0);

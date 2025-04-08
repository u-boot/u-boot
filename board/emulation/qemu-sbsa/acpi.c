// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 9elements GmbH
 */

#include <cpu.h>
#include <tables_csum.h>
#include <string.h>
#include <acpi/acpi_table.h>
#include <asm/acpi_table.h>
#include <asm/armv8/sec_firmware.h>
#include <configs/qemu-sbsa.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass.h>

#define SBSAQEMU_MADT_GIC_VBASE          0x2c020000
#define SBSAQEMU_MADT_GIC_HBASE          0x2c010000
#define SBSAQEMU_MADT_GIC_PMU_IRQ        23

#define SBSA_PLATFORM_WATCHDOG_COUNT    1
#define SBSA_PLATFORM_TIMER_COUNT       (SBSA_PLATFORM_WATCHDOG_COUNT)

#define L2_ATTRIBUTES (ACPI_PPTT_READ_ALLOC | ACPI_PPTT_WRITE_ALLOC | \
			(ACPI_PPTT_CACHE_TYPE_UNIFIED << \
			 ACPI_PPTT_CACHE_TYPE_SHIFT))
#define L2_SIZE 0x80000
#define L2_SETS 0x400
#define L2_WAYS 8

#define L1D_ATTRIBUTES (ACPI_PPTT_READ_ALLOC | ACPI_PPTT_WRITE_ALLOC | \
			(ACPI_PPTT_CACHE_TYPE_DATA << \
			 ACPI_PPTT_CACHE_TYPE_SHIFT))
#define L1D_SIZE 0x8000
#define L1D_SETS 0x100
#define L1D_WAYS 2

#define L1I_ATTRIBUTES (ACPI_PPTT_READ_ALLOC | \
			(ACPI_PPTT_CACHE_TYPE_INSTR << \
			 ACPI_PPTT_CACHE_TYPE_SHIFT))
#define L1I_SIZE 0x8000
#define L1I_SETS 0x100
#define L1I_WAYS 2

int acpi_fill_iort(struct acpi_ctx *ctx)
{
	u32 its_offset, smmu_offset;
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_IRQ,
					  DM_DRIVER_GET(arm_gic_v3_its), &dev);
	if (ret) {
		pr_err("%s: failed to get %s irq device\n", __func__,
		       DM_DRIVER_GET(arm_gic_v3_its)->name);
		return ret;
	}

	u32 identifiers[] = { dev_seq(dev) };

	its_offset = acpi_iort_add_its_group(ctx, ARRAY_SIZE(identifiers),
					     identifiers);

	struct acpi_iort_id_mapping map_smmu[] = {{
		0, 0xffff, 0, its_offset, 0
	}};

	smmu_offset = acpi_iort_add_smmu_v3(ctx,
					    SBSA_SMMU_BASE_ADDR, // Base address
					    ACPI_IORT_SMMU_V3_COHACC_OVERRIDE, // Flags
					    0,  // VATOS address
					    0,  // SMMUv3 Model
					    74, // Event
					    75, // Pri
					    77, // Gerror
					    76, // Sync
					    0,  // Proximity domain
					    1,  // DevIDMappingIndex
					    ARRAY_SIZE(map_smmu),
					    map_smmu);

	struct acpi_iort_id_mapping map_rc[] = {{
		0, 0xffff, 0, smmu_offset, 0
	}};

	acpi_iort_add_rc(ctx,
			 BIT(0) | BIT(56),  // CacheCoherent + CPM
			 0,  // AtsAttribute
			 0,  // PciSegmentNumber
			 64, // MemoryAddressSizeLimit
			 ARRAY_SIZE(map_rc),
			 map_rc);
	return 0;
}

void acpi_fill_fadt(struct acpi_fadt *fadt)
{
	fadt->flags = ACPI_FADT_HW_REDUCED_ACPI | ACPI_FADT_LOW_PWR_IDLE_S0;
	fadt->preferred_pm_profile = ACPI_PM_PERFORMANCE_SERVER;
	fadt->arm_boot_arch = ACPI_ARM_PSCI_COMPLIANT;
}

int acpi_fill_mcfg(struct acpi_ctx *ctx)
{
	size_t size;

	/* PCI Segment Group 0, Start Bus Number 0, End Bus Number is 255 */
	size = acpi_create_mcfg_mmconfig((void *)ctx->current,
					 SBSA_PCIE_ECAM_BASE_ADDR, 0, 0, 255);
	acpi_inc(ctx, size);

	return 0;
}

static int sbsa_write_gtdt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_gtdt *gtdt;

	gtdt = ctx->current;
	header = &gtdt->header;

	memset(gtdt, '\0', sizeof(struct acpi_gtdt));

	acpi_fill_header(header, "GTDT");
	header->length = sizeof(struct acpi_gtdt);
	header->revision = acpi_get_table_revision(ACPITAB_GTDT);

	gtdt->cnt_ctrl_base = 0xFFFFFFFFFFFFFFFF;
	gtdt->sec_el1_gsiv = 29;
	gtdt->sec_el1_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->el1_gsiv = 30;
	gtdt->el1_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->virt_el1_gsiv = 27;
	gtdt->virt_el1_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->el2_gsiv = 26;
	gtdt->el2_flags = GTDT_FLAG_INT_ACTIVE_LOW;
	gtdt->cnt_read_base = 0xffffffffffffffff;

	// FIXME: VirtualPL2Timer
	acpi_update_checksum(header);

	acpi_add_table(ctx, gtdt);

	acpi_inc(ctx, sizeof(struct acpi_gtdt));

	return 0;
};

ACPI_WRITER(5gtdt, "GTDT", sbsa_write_gtdt, 0);

static int acpi_write_pptt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	int cluster_offset, l2_offset;
	u32 offsets[2];

	header = ctx->current;
	ctx->tab_start = ctx->current;

	memset(header, '\0', sizeof(struct acpi_table_header));

	acpi_fill_header(header, "PPTT");
	header->revision = acpi_get_table_revision(ACPITAB_PPTT);
	acpi_inc(ctx, sizeof(*header));

	cluster_offset = acpi_pptt_add_proc(ctx, ACPI_PPTT_PHYSICAL_PACKAGE |
					    ACPI_PPTT_CHILDREN_IDENTICAL,
					    0, 0, 0, NULL);

	l2_offset = acpi_pptt_add_cache(ctx, ACPI_PPTT_ALL_VALID, 0, L2_SIZE,
					L2_SETS, L2_WAYS, L2_ATTRIBUTES, 64);

	offsets[0] = acpi_pptt_add_cache(ctx, ACPI_PPTT_ALL_VALID, l2_offset,
					 L1D_SIZE, L1D_SETS, L1D_WAYS,
					 L1D_ATTRIBUTES, 64);

	offsets[1] = acpi_pptt_add_cache(ctx, ACPI_PPTT_ALL_BUT_WRITE_POL,
					 l2_offset, L1I_SIZE, L1I_SETS,
					 L1I_WAYS, L1I_ATTRIBUTES, 64);

	for (int i = 0; i < uclass_id_count(UCLASS_CPU); i++) {
		acpi_pptt_add_proc(ctx, ACPI_PPTT_CHILDREN_IDENTICAL |
				   ACPI_PPTT_NODE_IS_LEAF | ACPI_PPTT_PROC_ID_VALID,
				   cluster_offset, i, 2, offsets);
	}

	header->length = ctx->current - ctx->tab_start;
	acpi_update_checksum(header);

	acpi_inc(ctx, header->length);
	acpi_add_table(ctx, header);

	return 0;
};

ACPI_WRITER(5pptt, "PPTT", acpi_write_pptt, 0);

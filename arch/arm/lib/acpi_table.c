// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2024 9elements GmbH
 */

#define LOG_CATEGORY LOGC_ACPI

#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_table.h>
#include <cpu_func.h>
#include <efi_loader.h>
#include <malloc.h>
#include <string.h>
#include <tables_csum.h>

void acpi_write_madt_gicc(struct acpi_madt_gicc *gicc, uint cpu_num,
			  uint perf_gsiv, ulong phys_base, ulong gicv,
			  ulong gich, uint vgic_maint_irq, ulong mpidr,
			  uint efficiency)
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

void acpi_write_parking_protocol(struct acpi_madt *madt)
{
	struct acpi_parking_protocol_page *page;
	struct acpi_madt_gicc *gicc;
	void *reloc_addr;
	u64 start;
	int ncpus = 0;

	/* According to the "Multi-processor Startup for ARM Platforms":
	 * - Every CPU as specified by MADT GICC has it's own 4K page
	 * - Every page is divided into two sections: OS and FW reserved
	 * - Memory occupied by "Parking Protocol" must be marked 'Reserved'
	 * - Spinloop code should reside in FW reserved 2048 bytes
	 * - Spinloop code will check the mailbox in OS reserved area
	 */

	if (acpi_parking_protocol_code_size > sizeof(page->cpu_spinning_code)) {
		log_err("Spinning code too big to fit: %d\n",
			acpi_parking_protocol_code_size);
	}

	/* Count all cores including BSP */
	for (int i = sizeof(struct acpi_madt); i < madt->header.length; ) {
		gicc = (struct acpi_madt_gicc *)((void *)madt + i);
		if (gicc->type != ACPI_APIC_GICC) {
			i += gicc->length;
			continue;
		}
		ncpus++;
		i += gicc->length;
	}
	debug("Found %d GICCs in MADT\n", ncpus);

	/* Allocate pages linearly due to assembly code requirements */
	page = memalign(ACPI_PP_PAGE_SIZE, ACPI_PP_PAGE_SIZE * ncpus);
	start = (u64)(uintptr_t)page;

#if CONFIG_IS_ENABLED(EFI_LOADER)
	int ret = efi_add_memory_map(start, ncpus * ACPI_PP_PAGE_SIZE, EFI_RESERVED_MEMORY_TYPE);

	if (ret != EFI_SUCCESS)
		log_err("Reserved memory mapping failed addr %llx size %x\n",
			start, ncpus * ACPI_PP_PAGE_SIZE);
#endif

	/* Prepare the parking protocol pages */
	for (int i = sizeof(struct acpi_madt); i < madt->header.length; ) {
		gicc = (struct acpi_madt_gicc *)((void *)madt + i);
		if (gicc->type != ACPI_APIC_GICC) {
			i += gicc->length;
			continue;
		}

		/* Update GICC */
		gicc->parking_proto = ACPI_PP_VERSION;
		gicc->parked_addr = (uint64_t)(uintptr_t)page;

		/* Prepare parking protocol page */
		memset(page, 0, sizeof(struct acpi_parking_protocol_page));
		page->cpu_id = ACPI_PP_CPU_ID_INVALID;
		page->jumping_address = ACPI_PP_JMP_ADR_INVALID;

		/* Relocate spinning code */
		reloc_addr = &page->cpu_spinning_code[0];

		debug("Relocating spin table from %p to %p (size %x)\n",
		      &acpi_parking_protocol_code_start, reloc_addr,
		      acpi_parking_protocol_code_size);
		memcpy(reloc_addr, &acpi_parking_protocol_code_start,
		       acpi_parking_protocol_code_size);

		if (!CONFIG_IS_ENABLED(SYS_DCACHE_OFF))
			flush_dcache_range((unsigned long)page,
					   (unsigned long)page +
					   sizeof(struct acpi_parking_protocol_page));
		page++;
		i += gicc->length;
	}

	/* Point secondary CPUs to new spin loop code */
	acpi_parking_protocol_install(start, ncpus);
}

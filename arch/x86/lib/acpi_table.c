/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0+
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/lists.h>
#include <asm/acpi_table.h>
#include <asm/cpu.h>
#include <asm/ioapic.h>
#include <asm/lapic.h>
#include <asm/tables.h>
#include <asm/pci.h>

/*
 * IASL compiles the dsdt entries and
 * writes the hex values to AmlCode array.
 * CamelCase cannot be handled here.
 */
extern const unsigned char AmlCode[];

/**
* Add an ACPI table to the RSDT (and XSDT) structure, recalculate length
* and checksum.
*/
static void acpi_add_table(struct acpi_rsdp *rsdp, void *table)
{
	int i, entries_num;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt = NULL;

	/* The RSDT is mandatory while the XSDT is not */
	rsdt = (struct acpi_rsdt *)rsdp->rsdt_address;

	if (rsdp->xsdt_address)
		xsdt = (struct acpi_xsdt *)((u32)rsdp->xsdt_address);

	/* This should always be MAX_ACPI_TABLES */
	entries_num = ARRAY_SIZE(rsdt->entry);

	for (i = 0; i < entries_num; i++) {
		if (rsdt->entry[i] == 0)
			break;
	}

	if (i >= entries_num) {
		debug("ACPI: Error: too many tables.\n");
		return;
	}

	/* Add table to the RSDT */
	rsdt->entry[i] = (u32)table;

	/* Fix RSDT length or the kernel will assume invalid entries */
	rsdt->header.length = sizeof(acpi_header_t) + (sizeof(u32) * (i + 1));

	/* Re-calculate checksum */
	rsdt->header.checksum = 0;
	rsdt->header.checksum = table_compute_checksum((u8 *)rsdt,
						       rsdt->header.length);

	/*
	 * And now the same thing for the XSDT. We use the same index as for
	 * now we want the XSDT and RSDT to always be in sync in U-Boot
	 */
	if (xsdt) {
		/* Add table to the XSDT */
		xsdt->entry[i] = (u64)(u32)table;

		/* Fix XSDT length */
		xsdt->header.length = sizeof(acpi_header_t) +
			 (sizeof(u64) * (i + 1));

		/* Re-calculate checksum */
		xsdt->header.checksum = 0;
		xsdt->header.checksum = table_compute_checksum((u8 *)xsdt,
				xsdt->header.length);
	}
}

static int acpi_create_madt_lapic(struct acpi_madt_lapic *lapic,
			 u8 cpu, u8 apic)
{
	lapic->type = LOCALAPIC; /* Local APIC structure */
	lapic->length = sizeof(struct acpi_madt_lapic);
	lapic->flags = LOCAL_APIC_FLAG_ENABLED; /* Processor/LAPIC enabled */
	lapic->processor_id = cpu;
	lapic->apic_id = apic;

	return lapic->length;
}

unsigned long acpi_create_madt_lapics(unsigned long current)
{
	struct udevice *dev;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);

		current += acpi_create_madt_lapic(
			(struct acpi_madt_lapic *)current,
			plat->cpu_id, plat->cpu_id);
		}
		return current;
}

int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic, u8 id, u32 addr,
			 u32 gsi_base)
{
	ioapic->type = IOAPIC;
	ioapic->length = sizeof(struct acpi_madt_ioapic);
	ioapic->reserved = 0x00;
	ioapic->gsi_base = gsi_base;
	ioapic->ioapic_id = id;
	ioapic->ioapic_addr = addr;

	return ioapic->length;
}

int acpi_create_madt_irqoverride(struct acpi_madt_irqoverride *irqoverride,
			 u8 bus, u8 source, u32 gsirq, u16 flags)
{
	irqoverride->type = IRQSOURCEOVERRIDE;
	irqoverride->length = sizeof(struct acpi_madt_irqoverride);
	irqoverride->bus = bus;
	irqoverride->source = source;
	irqoverride->gsirq = gsirq;
	irqoverride->flags = flags;

	return irqoverride->length;
}

int acpi_create_madt_lapic_nmi(struct acpi_madt_lapic_nmi *lapic_nmi,
			 u8 cpu, u16 flags, u8 lint)
{
	lapic_nmi->type = LOCALNMITYPE;
	lapic_nmi->length = sizeof(struct acpi_madt_lapic_nmi);
	lapic_nmi->flags = flags;
	lapic_nmi->processor_id = cpu;
	lapic_nmi->lint = lint;

	return lapic_nmi->length;
}

static void fill_header(acpi_header_t *header, char *signature, int length)
{
	memcpy(header->signature, signature, length);
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, ACPI_TABLE_CREATOR, 8);
	memcpy(header->asl_compiler_id, ASLC, 4);
}

static void acpi_create_madt(struct acpi_madt *madt)
{
	acpi_header_t *header = &(madt->header);
	unsigned long current = (unsigned long)madt + sizeof(struct acpi_madt);

	memset((void *)madt, 0, sizeof(struct acpi_madt));

	/* Fill out header fields */
	fill_header(header, "APIC", 4);
	header->length = sizeof(struct acpi_madt);

	/* ACPI 1.0/2.0: 1, ACPI 3.0: 2, ACPI 4.0: 3 */
	header->revision = ACPI_REV_ACPI_2_0;

	madt->lapic_addr = LAPIC_DEFAULT_BASE;
	madt->flags = PCAT_COMPAT;

	current = acpi_fill_madt(current);

	/* (Re)calculate length and checksum */
	header->length = current - (unsigned long)madt;

	header->checksum = table_compute_checksum((void *)madt, header->length);
}

static int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig,
			 u32 base, u16 seg_nr, u8 start, u8 end)
{
	memset(mmconfig, 0, sizeof(*mmconfig));
	mmconfig->base_address = base;
	mmconfig->base_reserved = 0;
	mmconfig->pci_segment_group_number = seg_nr;
	mmconfig->start_bus_number = start;
	mmconfig->end_bus_number = end;

	return sizeof(struct acpi_mcfg_mmconfig);
}

static unsigned long acpi_fill_mcfg(unsigned long current)
{
	current += acpi_create_mcfg_mmconfig
		((struct acpi_mcfg_mmconfig *)current,
		 CONFIG_PCIE_ECAM_BASE, 0x0, 0x0, 255);

	return current;
}

/* MCFG is defined in the PCI Firmware Specification 3.0 */
static void acpi_create_mcfg(struct acpi_mcfg *mcfg)
{
	acpi_header_t *header = &(mcfg->header);
	unsigned long current = (unsigned long)mcfg + sizeof(struct acpi_mcfg);

	memset((void *)mcfg, 0, sizeof(struct acpi_mcfg));

	/* Fill out header fields */
	fill_header(header, "MCFG", 4);
	header->length = sizeof(struct acpi_mcfg);

	/* ACPI 1.0/2.0: 1, ACPI 3.0: 2, ACPI 4.0: 3 */
	header->revision = ACPI_REV_ACPI_2_0;

	current = acpi_fill_mcfg(current);

	/* (Re)calculate length and checksum */
	header->length = current - (unsigned long)mcfg;
	header->checksum = table_compute_checksum((void *)mcfg, header->length);
}

static void acpi_create_facs(struct acpi_facs *facs)
{
	memset((void *)facs, 0, sizeof(struct acpi_facs));

	memcpy(facs->signature, "FACS", 4);
	facs->length = sizeof(struct acpi_facs);
	facs->hardware_signature = 0;
	facs->firmware_waking_vector = 0;
	facs->global_lock = 0;
	facs->flags = 0;
	facs->x_firmware_waking_vector_l = 0;
	facs->x_firmware_waking_vector_h = 0;
	facs->version = 1; /* ACPI 1.0: 0, ACPI 2.0/3.0: 1, ACPI 4.0: 2 */
}

static void acpi_write_rsdt(struct acpi_rsdt *rsdt)
{
	acpi_header_t *header = &(rsdt->header);

	/* Fill out header fields */
	fill_header(header, "RSDT", 4);
	header->length = sizeof(struct acpi_rsdt);

	/* ACPI 1.0/2.0: 1, ACPI 3.0: 2, ACPI 4.0: 3 */
	header->revision = ACPI_REV_ACPI_2_0;

	/* Entries are filled in later, we come with an empty set */

	/* Fix checksum */
	header->checksum = table_compute_checksum((void *)rsdt,
			sizeof(struct acpi_rsdt));
}

static void acpi_write_xsdt(struct acpi_xsdt *xsdt)
{
	acpi_header_t *header = &(xsdt->header);

	/* Fill out header fields */
	fill_header(header, "XSDT", 4);
	header->length = sizeof(struct acpi_xsdt);

	/* ACPI 1.0/2.0: 1, ACPI 3.0: 2, ACPI 4.0: 3 */
	header->revision = ACPI_REV_ACPI_2_0;

	/* Entries are filled in later, we come with an empty set */

	/* Fix checksum */
	header->checksum = table_compute_checksum((void *)xsdt,
			sizeof(struct acpi_xsdt));
}

static void acpi_write_rsdp(struct acpi_rsdp *rsdp, struct acpi_rsdt *rsdt,
			 struct acpi_xsdt *xsdt)
{
	memset(rsdp, 0, sizeof(struct acpi_rsdp));

	memcpy(rsdp->signature, RSDP_SIG, 8);
	memcpy(rsdp->oem_id, OEM_ID, 6);

	rsdp->length = sizeof(struct acpi_rsdp);
	rsdp->rsdt_address = (u32)rsdt;

	/*
	* Revision: ACPI 1.0: 0, ACPI 2.0/3.0/4.0: 2
	*
	* Some OSes expect an XSDT to be present for RSD PTR revisions >= 2.
	* If we don't have an ACPI XSDT, force ACPI 1.0 (and thus RSD PTR
	* revision 0)
	*/
	if (xsdt == NULL) {
		rsdp->revision = ACPI_RSDP_REV_ACPI_1_0;
	} else {
		rsdp->xsdt_address = (u64)(u32)xsdt;
		rsdp->revision = ACPI_RSDP_REV_ACPI_2_0;
	}

	/* Calculate checksums */
	rsdp->checksum = table_compute_checksum((void *)rsdp, 20);
	rsdp->ext_checksum = table_compute_checksum((void *)rsdp,
			sizeof(struct acpi_rsdp));
}

static void acpi_create_ssdt_generator(acpi_header_t *ssdt,
			 const char *oem_table_id)
{
	unsigned long current = (unsigned long)ssdt + sizeof(acpi_header_t);

	memset((void *)ssdt, 0, sizeof(acpi_header_t));

	memcpy(&ssdt->signature, "SSDT", 4);
	/* Access size in ACPI 2.0c/3.0/4.0/5.0 */
	ssdt->revision = ACPI_REV_ACPI_3_0;
	memcpy(&ssdt->oem_id, OEM_ID, 6);
	memcpy(&ssdt->oem_table_id, oem_table_id, 8);
	ssdt->oem_revision = OEM_REVISION;
	memcpy(&ssdt->asl_compiler_id, ASLC, 4);
	ssdt->asl_compiler_revision = ASL_COMPILER_REVISION;
	ssdt->length = sizeof(acpi_header_t);

	/* (Re)calculate length and checksum */
	ssdt->length = current - (unsigned long)ssdt;
	ssdt->checksum = table_compute_checksum((void *)ssdt, ssdt->length);
}

/*
 * QEMU's version of write_acpi_tables is defined in
 * arch/x86/cpu/qemu/fw_cfg.c
 */
unsigned long write_acpi_tables(unsigned long start)
{
	unsigned long current;
	struct acpi_rsdp *rsdp;
	struct acpi_rsdt *rsdt;
	struct acpi_xsdt *xsdt;
	struct acpi_facs *facs;
	acpi_header_t *dsdt;
	struct acpi_fadt *fadt;
	struct acpi_mcfg *mcfg;
	struct acpi_madt *madt;
	acpi_header_t *ssdt;

	current = start;

	/* Align ACPI tables to 16byte */
	current = ALIGN(current, 16);

	debug("ACPI: Writing ACPI tables at %lx.\n", start);

	/* We need at least an RSDP and an RSDT Table */
	rsdp = (struct acpi_rsdp *)current;
	current += sizeof(struct acpi_rsdp);
	current = ALIGN(current, 16);
	rsdt = (struct acpi_rsdt *)current;
	current += sizeof(struct acpi_rsdt);
	current = ALIGN(current, 16);
	xsdt = (struct acpi_xsdt *)current;
	current += sizeof(struct acpi_xsdt);
	current = ALIGN(current, 16);

	/* clear all table memory */
	memset((void *)start, 0, current - start);

	acpi_write_rsdp(rsdp, rsdt, xsdt);
	acpi_write_rsdt(rsdt);
	acpi_write_xsdt(xsdt);

	debug("ACPI:    * FACS\n");
	facs = (struct acpi_facs *)current;
	current += sizeof(struct acpi_facs);
	current = ALIGN(current, 16);

	acpi_create_facs(facs);

	debug("ACPI:    * DSDT\n");
	dsdt = (acpi_header_t *)current;
	memcpy(dsdt, &AmlCode, sizeof(acpi_header_t));
	if (dsdt->length >= sizeof(acpi_header_t)) {
		current += sizeof(acpi_header_t);
		memcpy((char *)current,
				(char *)&AmlCode + sizeof(acpi_header_t),
				dsdt->length - sizeof(acpi_header_t));
		current += dsdt->length - sizeof(acpi_header_t);

		/* (Re)calculate length and checksum */
		dsdt->length = current - (unsigned long)dsdt;
		dsdt->checksum = 0;
		dsdt->checksum = table_compute_checksum((void *)dsdt,
				dsdt->length);
	}
	current = ALIGN(current, 16);

	debug("ACPI:    * FADT\n");
	fadt = (struct acpi_fadt *)current;
	current += sizeof(struct acpi_fadt);
	current = ALIGN(current, 16);
	acpi_create_fadt(fadt, facs, dsdt);
	acpi_add_table(rsdp, fadt);

	debug("ACPI:    * MCFG\n");
	mcfg = (struct acpi_mcfg *)current;
	acpi_create_mcfg(mcfg);
	if (mcfg->header.length > sizeof(struct acpi_mcfg)) {
		current += mcfg->header.length;
		current = ALIGN(current, 16);
		acpi_add_table(rsdp, mcfg);
	}

	debug("ACPI:    * MADT\n");
	madt = (struct acpi_madt *)current;
	acpi_create_madt(madt);
	if (madt->header.length > sizeof(struct acpi_madt)) {
		current += madt->header.length;
		acpi_add_table(rsdp, madt);
	}
	current = ALIGN(current, 16);

	debug("ACPI:    * SSDT\n");
	ssdt = (acpi_header_t *)current;
	acpi_create_ssdt_generator(ssdt, ACPI_TABLE_CREATOR);
	if (ssdt->length > sizeof(acpi_header_t)) {
		current += ssdt->length;
		acpi_add_table(rsdp, ssdt);
		current = ALIGN(current, 16);
	}

	debug("current = %lx\n", current);

	debug("ACPI: done.\n");

	return current;
}

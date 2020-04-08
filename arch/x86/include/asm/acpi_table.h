/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#ifndef __ASM_ACPI_TABLE_H__
#define __ASM_ACPI_TABLE_H__

struct acpi_facs;
struct acpi_fadt;
struct acpi_global_nvs;
struct acpi_madt_ioapic;
struct acpi_madt_irqoverride;
struct acpi_madt_lapic_nmi;
struct acpi_mcfg_mmconfig;
struct acpi_table_header;

/* These can be used by the target port */

void acpi_fill_header(struct acpi_table_header *header, char *signature);
void acpi_create_fadt(struct acpi_fadt *fadt, struct acpi_facs *facs,
		      void *dsdt);
int acpi_create_madt_lapics(u32 current);
int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic, u8 id,
			    u32 addr, u32 gsi_base);
int acpi_create_madt_irqoverride(struct acpi_madt_irqoverride *irqoverride,
				 u8 bus, u8 source, u32 gsirq, u16 flags);
int acpi_create_madt_lapic_nmi(struct acpi_madt_lapic_nmi *lapic_nmi,
			       u8 cpu, u16 flags, u8 lint);
u32 acpi_fill_madt(u32 current);
int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u32 base,
			      u16 seg_nr, u8 start, u8 end);
u32 acpi_fill_mcfg(u32 current);
u32 acpi_fill_csrt(u32 current);
void acpi_create_gnvs(struct acpi_global_nvs *gnvs);
ulong write_acpi_tables(ulong start);

/**
 * acpi_get_rsdp_addr() - get ACPI RSDP table address
 *
 * This routine returns the ACPI RSDP table address in the system memory.
 *
 * @return:	ACPI RSDP table address
 */
ulong acpi_get_rsdp_addr(void);

#endif /* __ASM_ACPI_TABLE_H__ */

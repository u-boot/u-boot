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

/**
 * acpi_write_hpet() - Write out a HPET table
 *
 * Write out the table for High-Precision Event Timers
 *
 * @ctx: Current ACPI context
 * @return 0 if OK, -ve on error
 */
int acpi_write_hpet(struct acpi_ctx *ctx);

/**
 * acpi_write_dbg2_pci_uart() - Write out a DBG2 table
 *
 * @ctx: Current ACPI context
 * @dev: Debug UART device to describe
 * @access_size: Access size for UART (e.g. ACPI_ACCESS_SIZE_DWORD_ACCESS)
 * @return 0 if OK, -ve on error
 */
int acpi_write_dbg2_pci_uart(struct acpi_ctx *ctx, struct udevice *dev,
			     uint access_size);

/**
 * acpi_create_gnvs() - Create a GNVS (Global Non Volatile Storage) table
 *
 * @gnvs: Table to fill in
 * @return 0 if OK, -ve on error
 */
int acpi_create_gnvs(struct acpi_global_nvs *gnvs);

ulong write_acpi_tables(ulong start);

/**
 * acpi_get_rsdp_addr() - get ACPI RSDP table address
 *
 * This routine returns the ACPI RSDP table address in the system memory.
 *
 * @return:	ACPI RSDP table address
 */
ulong acpi_get_rsdp_addr(void);

/**
 * arch_read_sci_irq_select() - Read the system-control interrupt number
 *
 * @returns value of IRQ register in the PMC
 */
int arch_read_sci_irq_select(void);

/**
 * arch_write_sci_irq_select() - Set the system-control interrupt number
 *
 * @scis: New value for IRQ register in the PMC
 */
int arch_write_sci_irq_select(uint scis);

/**
 * arch_madt_sci_irq_polarity() - Return the priority to use for the MADT
 *
 * @sci: System-control interrupt number
 * @return priority to use (MP_IRQ_POLARITY_...)
 */
int arch_madt_sci_irq_polarity(int sci);

#endif /* __ASM_ACPI_TABLE_H__ */

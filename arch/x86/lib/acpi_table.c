// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <dm/uclass-internal.h>
#include <mapmem.h>
#include <serial.h>
#include <version.h>
#include <acpi/acpi_table.h>
#include <asm/acpi/global_nvs.h>
#include <asm/ioapic.h>
#include <asm/lapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/arch/global_nvs.h>
#include <dm/acpi.h>

/*
 * IASL compiles the dsdt entries and writes the hex values
 * to a C array AmlCode[] (see dsdt.c).
 */
extern const unsigned char AmlCode[];

/* ACPI RSDP address to be used in boot parameters */
static ulong acpi_rsdp_addr;

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
	facs->version = 1;
}

static int acpi_create_madt_lapic(struct acpi_madt_lapic *lapic,
				  u8 cpu, u8 apic)
{
	lapic->type = ACPI_APIC_LAPIC;
	lapic->length = sizeof(struct acpi_madt_lapic);
	lapic->flags = LOCAL_APIC_FLAG_ENABLED;
	lapic->processor_id = cpu;
	lapic->apic_id = apic;

	return lapic->length;
}

int acpi_create_madt_lapics(u32 current)
{
	struct udevice *dev;
	int total_length = 0;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_platdata *plat = dev_get_parent_platdata(dev);
		int length = acpi_create_madt_lapic(
				(struct acpi_madt_lapic *)current,
				plat->cpu_id, plat->cpu_id);
		current += length;
		total_length += length;
	}

	return total_length;
}

int acpi_create_madt_ioapic(struct acpi_madt_ioapic *ioapic, u8 id,
			    u32 addr, u32 gsi_base)
{
	ioapic->type = ACPI_APIC_IOAPIC;
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
	irqoverride->type = ACPI_APIC_IRQ_SRC_OVERRIDE;
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
	lapic_nmi->type = ACPI_APIC_LAPIC_NMI;
	lapic_nmi->length = sizeof(struct acpi_madt_lapic_nmi);
	lapic_nmi->flags = flags;
	lapic_nmi->processor_id = cpu;
	lapic_nmi->lint = lint;

	return lapic_nmi->length;
}

static int acpi_create_madt_irq_overrides(u32 current)
{
	struct acpi_madt_irqoverride *irqovr;
	u16 sci_flags = MP_IRQ_TRIGGER_LEVEL | MP_IRQ_POLARITY_HIGH;
	int length = 0;

	irqovr = (void *)current;
	length += acpi_create_madt_irqoverride(irqovr, 0, 0, 2, 0);

	irqovr = (void *)(current + length);
	length += acpi_create_madt_irqoverride(irqovr, 0, 9, 9, sci_flags);

	return length;
}

__weak u32 acpi_fill_madt(u32 current)
{
	current += acpi_create_madt_lapics(current);

	current += acpi_create_madt_ioapic((struct acpi_madt_ioapic *)current,
			io_apic_read(IO_APIC_ID) >> 24, IO_APIC_ADDR, 0);

	current += acpi_create_madt_irq_overrides(current);

	return current;
}

static void acpi_create_madt(struct acpi_madt *madt)
{
	struct acpi_table_header *header = &(madt->header);
	u32 current = (u32)madt + sizeof(struct acpi_madt);

	memset((void *)madt, 0, sizeof(struct acpi_madt));

	/* Fill out header fields */
	acpi_fill_header(header, "APIC");
	header->length = sizeof(struct acpi_madt);
	header->revision = 4;

	madt->lapic_addr = LAPIC_DEFAULT_BASE;
	madt->flags = ACPI_MADT_PCAT_COMPAT;

	current = acpi_fill_madt(current);

	/* (Re)calculate length and checksum */
	header->length = current - (u32)madt;

	header->checksum = table_compute_checksum((void *)madt, header->length);
}

int acpi_create_mcfg_mmconfig(struct acpi_mcfg_mmconfig *mmconfig, u32 base,
			      u16 seg_nr, u8 start, u8 end)
{
	memset(mmconfig, 0, sizeof(*mmconfig));
	mmconfig->base_address_l = base;
	mmconfig->base_address_h = 0;
	mmconfig->pci_segment_group_number = seg_nr;
	mmconfig->start_bus_number = start;
	mmconfig->end_bus_number = end;

	return sizeof(struct acpi_mcfg_mmconfig);
}

__weak u32 acpi_fill_mcfg(u32 current)
{
	current += acpi_create_mcfg_mmconfig
		((struct acpi_mcfg_mmconfig *)current,
		CONFIG_PCIE_ECAM_BASE, 0x0, 0x0, 255);

	return current;
}

/* MCFG is defined in the PCI Firmware Specification 3.0 */
static void acpi_create_mcfg(struct acpi_mcfg *mcfg)
{
	struct acpi_table_header *header = &(mcfg->header);
	u32 current = (u32)mcfg + sizeof(struct acpi_mcfg);

	memset((void *)mcfg, 0, sizeof(struct acpi_mcfg));

	/* Fill out header fields */
	acpi_fill_header(header, "MCFG");
	header->length = sizeof(struct acpi_mcfg);
	header->revision = 1;

	current = acpi_fill_mcfg(current);

	/* (Re)calculate length and checksum */
	header->length = current - (u32)mcfg;
	header->checksum = table_compute_checksum((void *)mcfg, header->length);
}

__weak u32 acpi_fill_csrt(u32 current)
{
	return current;
}

static void acpi_create_csrt(struct acpi_csrt *csrt)
{
	struct acpi_table_header *header = &(csrt->header);
	u32 current = (u32)csrt + sizeof(struct acpi_csrt);

	memset((void *)csrt, 0, sizeof(struct acpi_csrt));

	/* Fill out header fields */
	acpi_fill_header(header, "CSRT");
	header->length = sizeof(struct acpi_csrt);
	header->revision = 0;

	current = acpi_fill_csrt(current);

	/* (Re)calculate length and checksum */
	header->length = current - (u32)csrt;
	header->checksum = table_compute_checksum((void *)csrt, header->length);
}

static void acpi_create_spcr(struct acpi_spcr *spcr)
{
	struct acpi_table_header *header = &(spcr->header);
	struct serial_device_info serial_info = {0};
	ulong serial_address, serial_offset;
	struct udevice *dev;
	uint serial_config;
	uint serial_width;
	int access_size;
	int space_id;
	int ret = -ENODEV;

	/* Fill out header fields */
	acpi_fill_header(header, "SPCR");
	header->length = sizeof(struct acpi_spcr);
	header->revision = 2;

	/* Read the device once, here. It is reused below */
	dev = gd->cur_serial_dev;
	if (dev)
		ret = serial_getinfo(dev, &serial_info);
	if (ret)
		serial_info.type = SERIAL_CHIP_UNKNOWN;

	/* Encode chip type */
	switch (serial_info.type) {
	case SERIAL_CHIP_16550_COMPATIBLE:
		spcr->interface_type = ACPI_DBG2_16550_COMPATIBLE;
		break;
	case SERIAL_CHIP_UNKNOWN:
	default:
		spcr->interface_type = ACPI_DBG2_UNKNOWN;
		break;
	}

	/* Encode address space */
	switch (serial_info.addr_space) {
	case SERIAL_ADDRESS_SPACE_MEMORY:
		space_id = ACPI_ADDRESS_SPACE_MEMORY;
		break;
	case SERIAL_ADDRESS_SPACE_IO:
	default:
		space_id = ACPI_ADDRESS_SPACE_IO;
		break;
	}

	serial_width = serial_info.reg_width * 8;
	serial_offset = serial_info.reg_offset << serial_info.reg_shift;
	serial_address = serial_info.addr + serial_offset;

	/* Encode register access size */
	switch (serial_info.reg_shift) {
	case 0:
		access_size = ACPI_ACCESS_SIZE_BYTE_ACCESS;
		break;
	case 1:
		access_size = ACPI_ACCESS_SIZE_WORD_ACCESS;
		break;
	case 2:
		access_size = ACPI_ACCESS_SIZE_DWORD_ACCESS;
		break;
	case 3:
		access_size = ACPI_ACCESS_SIZE_QWORD_ACCESS;
		break;
	default:
		access_size = ACPI_ACCESS_SIZE_UNDEFINED;
		break;
	}

	debug("UART type %u @ %lx\n", spcr->interface_type, serial_address);

	/* Fill GAS */
	spcr->serial_port.space_id = space_id;
	spcr->serial_port.bit_width = serial_width;
	spcr->serial_port.bit_offset = 0;
	spcr->serial_port.access_size = access_size;
	spcr->serial_port.addrl = lower_32_bits(serial_address);
	spcr->serial_port.addrh = upper_32_bits(serial_address);

	/* Encode baud rate */
	switch (serial_info.baudrate) {
	case 9600:
		spcr->baud_rate = 3;
		break;
	case 19200:
		spcr->baud_rate = 4;
		break;
	case 57600:
		spcr->baud_rate = 6;
		break;
	case 115200:
		spcr->baud_rate = 7;
		break;
	default:
		spcr->baud_rate = 0;
		break;
	}

	serial_config = SERIAL_DEFAULT_CONFIG;
	if (dev)
		ret = serial_getconfig(dev, &serial_config);

	spcr->parity = SERIAL_GET_PARITY(serial_config);
	spcr->stop_bits = SERIAL_GET_STOP(serial_config);

	/* No PCI devices for now */
	spcr->pci_device_id = 0xffff;
	spcr->pci_vendor_id = 0xffff;

	/*
	 * SPCR has no clue if the UART base clock speed is different
	 * to the default one. However, the SPCR 1.04 defines baud rate
	 * 0 as a preconfigured state of UART and OS is supposed not
	 * to touch the configuration of the serial device.
	 */
	if (serial_info.clock != SERIAL_DEFAULT_CLOCK)
		spcr->baud_rate = 0;

	/* Fix checksum */
	header->checksum = table_compute_checksum((void *)spcr, header->length);
}

/*
 * QEMU's version of write_acpi_tables is defined in drivers/misc/qfw.c
 */
ulong write_acpi_tables(ulong start_addr)
{
	struct acpi_ctx sctx, *ctx = &sctx;
	struct acpi_facs *facs;
	struct acpi_table_header *dsdt;
	struct acpi_fadt *fadt;
	struct acpi_mcfg *mcfg;
	struct acpi_madt *madt;
	struct acpi_csrt *csrt;
	struct acpi_spcr *spcr;
	void *start;
	ulong addr;
	int i;

	start = map_sysmem(start_addr, 0);

	debug("ACPI: Writing ACPI tables at %lx\n", start_addr);

	acpi_setup_base_tables(ctx, start);

	debug("ACPI:    * FACS\n");
	facs = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_facs));

	acpi_create_facs(facs);

	debug("ACPI:    * DSDT\n");
	dsdt = ctx->current;
	memcpy(dsdt, &AmlCode, sizeof(struct acpi_table_header));
	acpi_inc(ctx, sizeof(struct acpi_table_header));
	memcpy(ctx->current,
	       (char *)&AmlCode + sizeof(struct acpi_table_header),
	       dsdt->length - sizeof(struct acpi_table_header));
	acpi_inc_align(ctx, dsdt->length - sizeof(struct acpi_table_header));

	/* Pack GNVS into the ACPI table area */
	for (i = 0; i < dsdt->length; i++) {
		u32 *gnvs = (u32 *)((u32)dsdt + i);
		if (*gnvs == ACPI_GNVS_ADDR) {
			ulong addr = (ulong)map_to_sysmem(ctx->current);

			debug("Fix up global NVS in DSDT to %#08lx\n", addr);
			*gnvs = addr;
			break;
		}
	}

	/* Update DSDT checksum since we patched the GNVS address */
	dsdt->checksum = 0;
	dsdt->checksum = table_compute_checksum((void *)dsdt, dsdt->length);

	/* Fill in platform-specific global NVS variables */
	acpi_create_gnvs(ctx->current);
	acpi_inc_align(ctx, sizeof(struct acpi_global_nvs));

	debug("ACPI:    * FADT\n");
	fadt = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_fadt));
	acpi_create_fadt(fadt, facs, dsdt);
	acpi_add_table(ctx, fadt);

	debug("ACPI:    * MADT\n");
	madt = ctx->current;
	acpi_create_madt(madt);
	acpi_inc_align(ctx, madt->header.length);
	acpi_add_table(ctx, madt);

	debug("ACPI:    * MCFG\n");
	mcfg = ctx->current;
	acpi_create_mcfg(mcfg);
	acpi_inc_align(ctx, mcfg->header.length);
	acpi_add_table(ctx, mcfg);

	debug("ACPI:    * CSRT\n");
	csrt = ctx->current;
	acpi_create_csrt(csrt);
	acpi_inc_align(ctx, csrt->header.length);
	acpi_add_table(ctx, csrt);

	debug("ACPI:    * SPCR\n");
	spcr = ctx->current;
	acpi_create_spcr(spcr);
	acpi_inc_align(ctx, spcr->header.length);
	acpi_add_table(ctx, spcr);

	acpi_write_dev_tables(ctx);

	addr = map_to_sysmem(ctx->current);
	debug("current = %lx\n", addr);

	acpi_rsdp_addr = (unsigned long)ctx->rsdp;
	debug("ACPI: done\n");

	return addr;
}

ulong acpi_get_rsdp_addr(void)
{
	return acpi_rsdp_addr;
}

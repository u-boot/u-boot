// SPDX-License-Identifier: GPL-2.0+
/*
 * Based on acpi.c from coreboot
 *
 * Copyright (C) 2015, Saket Sinha <saket.sinha89@gmail.com>
 * Copyright (C) 2016, Bin Meng <bmeng.cn@gmail.com>
 */

#define LOG_CATEGORY LOGC_ACPI

#include <common.h>
#include <bloblist.h>
#include <cpu.h>
#include <dm.h>
#include <log.h>
#include <dm/uclass-internal.h>
#include <mapmem.h>
#include <serial.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpi_table.h>
#include <asm/acpi/global_nvs.h>
#include <asm/ioapic.h>
#include <asm/global_data.h>
#include <asm/lapic.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/arch/global_nvs.h>
#include <dm/acpi.h>
#include <linux/err.h>

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
	int cpu_num = 0;

	for (uclass_find_first_device(UCLASS_CPU, &dev);
	     dev;
	     uclass_find_next_device(&dev)) {
		struct cpu_plat *plat = dev_get_parent_plat(dev);
		int length;

		length = acpi_create_madt_lapic(
			(struct acpi_madt_lapic *)current, cpu_num++,
			plat->cpu_id);
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
	header->revision = ACPI_MADT_REV_ACPI_3_0;

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

/**
 * acpi_create_tcpa() - Create a TCPA table
 *
 * @tcpa: Pointer to place to put table
 *
 * Trusted Computing Platform Alliance Capabilities Table
 * TCPA PC Specific Implementation SpecificationTCPA is defined in the PCI
 * Firmware Specification 3.0
 */
static int acpi_create_tcpa(struct acpi_tcpa *tcpa)
{
	struct acpi_table_header *header = &tcpa->header;
	u32 current = (u32)tcpa + sizeof(struct acpi_tcpa);
	int size = 0x10000;	/* Use this as the default size */
	void *log;
	int ret;

	if (!CONFIG_IS_ENABLED(BLOBLIST))
		return -ENXIO;
	memset(tcpa, '\0', sizeof(struct acpi_tcpa));

	/* Fill out header fields */
	acpi_fill_header(header, "TCPA");
	header->length = sizeof(struct acpi_tcpa);
	header->revision = 1;

	ret = bloblist_ensure_size_ret(BLOBLISTT_TCPA_LOG, &size, &log);
	if (ret)
		return log_msg_ret("blob", ret);

	tcpa->platform_class = 0;
	tcpa->laml = size;
	tcpa->lasa = (ulong)log;

	/* (Re)calculate length and checksum */
	header->length = current - (u32)tcpa;
	header->checksum = table_compute_checksum((void *)tcpa, header->length);

	return 0;
}

static int get_tpm2_log(void **ptrp, int *sizep)
{
	const int tpm2_default_log_len = 0x10000;
	int size;
	int ret;

	*sizep = 0;
	size = tpm2_default_log_len;
	ret = bloblist_ensure_size_ret(BLOBLISTT_TPM2_TCG_LOG, &size, ptrp);
	if (ret)
		return log_msg_ret("blob", ret);
	*sizep = size;

	return 0;
}

static int acpi_create_tpm2(struct acpi_tpm2 *tpm2)
{
	struct acpi_table_header *header = &tpm2->header;
	int tpm2_log_len;
	void *lasa;
	int ret;

	memset((void *)tpm2, 0, sizeof(struct acpi_tpm2));

	/*
	 * Some payloads like SeaBIOS depend on log area to use TPM2.
	 * Get the memory size and address of TPM2 log area or initialize it.
	 */
	ret = get_tpm2_log(&lasa, &tpm2_log_len);
	if (ret)
		return ret;

	/* Fill out header fields. */
	acpi_fill_header(header, "TPM2");
	memcpy(header->aslc_id, ASLC_ID, 4);

	header->length = sizeof(struct acpi_tpm2);
	header->revision = acpi_get_table_revision(ACPITAB_TPM2);

	/* Hard to detect for coreboot. Just set it to 0 */
	tpm2->platform_class = 0;

	/* Must be set to 0 for FIFO-interface support */
	tpm2->control_area = 0;
	tpm2->start_method = 6;
	memset(tpm2->msp, 0, sizeof(tpm2->msp));

	/* Fill the log area size and start address fields. */
	tpm2->laml = tpm2_log_len;
	tpm2->lasa = (uintptr_t)lasa;

	/* Calculate checksum. */
	header->checksum = table_compute_checksum((void *)tpm2, header->length);

	return 0;
}

__weak u32 acpi_fill_csrt(u32 current)
{
	return 0;
}

static int acpi_create_csrt(struct acpi_csrt *csrt)
{
	struct acpi_table_header *header = &(csrt->header);
	u32 current = (u32)csrt + sizeof(struct acpi_csrt);
	uint ptr;

	memset((void *)csrt, 0, sizeof(struct acpi_csrt));

	/* Fill out header fields */
	acpi_fill_header(header, "CSRT");
	header->length = sizeof(struct acpi_csrt);
	header->revision = 0;

	ptr = acpi_fill_csrt(current);
	if (!ptr)
		return -ENOENT;
	current = ptr;

	/* (Re)calculate length and checksum */
	header->length = current - (u32)csrt;
	header->checksum = table_compute_checksum((void *)csrt, header->length);

	return 0;
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

	memset((void *)spcr, 0, sizeof(struct acpi_spcr));

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

static int acpi_create_ssdt(struct acpi_ctx *ctx,
			    struct acpi_table_header *ssdt,
			    const char *oem_table_id)
{
	memset((void *)ssdt, '\0', sizeof(struct acpi_table_header));

	acpi_fill_header(ssdt, "SSDT");
	ssdt->revision = acpi_get_table_revision(ACPITAB_SSDT);
	ssdt->aslc_revision = 1;
	ssdt->length = sizeof(struct acpi_table_header);

	acpi_inc(ctx, sizeof(struct acpi_table_header));

	acpi_fill_ssdt(ctx);

	/* (Re)calculate length and checksum */
	ssdt->length = ctx->current - (void *)ssdt;
	ssdt->checksum = table_compute_checksum((void *)ssdt, ssdt->length);
	log_debug("SSDT at %p, length %x\n", ssdt, ssdt->length);

	/* Drop the table if it is empty */
	if (ssdt->length == sizeof(struct acpi_table_header)) {
		ctx->current = ssdt;
		return -ENOENT;
	}
	acpi_align(ctx);

	return 0;
}

/*
 * QEMU's version of write_acpi_tables is defined in drivers/misc/qfw.c
 */
ulong write_acpi_tables(ulong start_addr)
{
	const int thl = sizeof(struct acpi_table_header);
	struct acpi_ctx *ctx;
	struct acpi_facs *facs;
	struct acpi_table_header *dsdt;
	struct acpi_fadt *fadt;
	struct acpi_table_header *ssdt;
	struct acpi_mcfg *mcfg;
	struct acpi_tcpa *tcpa;
	struct acpi_madt *madt;
	struct acpi_csrt *csrt;
	struct acpi_spcr *spcr;
	void *start;
	int aml_len;
	ulong addr;
	int ret;
	int i;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return log_msg_ret("mem", -ENOMEM);
	gd->acpi_ctx = ctx;

	start = map_sysmem(start_addr, 0);

	debug("ACPI: Writing ACPI tables at %lx\n", start_addr);

	acpi_reset_items();
	acpi_setup_base_tables(ctx, start);

	debug("ACPI:    * FACS\n");
	facs = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_facs));

	acpi_create_facs(facs);

	debug("ACPI:    * DSDT\n");
	dsdt = ctx->current;

	/* Put the table header first */
	memcpy(dsdt, &AmlCode, thl);
	acpi_inc(ctx, thl);
	log_debug("DSDT starts at %p, hdr ends at %p\n", dsdt, ctx->current);

	/* If the table is not empty, allow devices to inject things */
	aml_len = dsdt->length - thl;
	if (aml_len) {
		void *base = ctx->current;

		acpi_inject_dsdt(ctx);
		log_debug("Added %x bytes from inject_dsdt, now at %p\n",
			  ctx->current - base, ctx->current);
		log_debug("Copy AML code size %x to %p\n", aml_len,
			  ctx->current);
		memcpy(ctx->current, AmlCode + thl, aml_len);
		acpi_inc(ctx, aml_len);
	}

	dsdt->length = ctx->current - (void *)dsdt;
	acpi_align(ctx);
	log_debug("Updated DSDT length to %x, total %x\n", dsdt->length,
		  ctx->current - (void *)dsdt);

	if (!IS_ENABLED(CONFIG_ACPI_GNVS_EXTERNAL)) {
		/* Pack GNVS into the ACPI table area */
		for (i = 0; i < dsdt->length; i++) {
			u32 *gnvs = (u32 *)((u32)dsdt + i);

			if (*gnvs == ACPI_GNVS_ADDR) {
				*gnvs = map_to_sysmem(ctx->current);
				debug("Fix up global NVS in DSDT to %#08x\n",
				      *gnvs);
				break;
			}
		}

		/*
		 * Fill in platform-specific global NVS variables. If this fails
		 * we cannot return the error but this should only happen while
		 * debugging.
		 */
		addr = acpi_create_gnvs(ctx->current);
		if (IS_ERR_VALUE(addr))
			printf("Error: Gailed to create GNVS\n");
		acpi_inc_align(ctx, sizeof(struct acpi_global_nvs));
	}

	/*
	 * Recalculate the length and update the DSDT checksum since we patched
	 * the GNVS address. Set the checksum to zero since it is part of the
	 * region being checksummed.
	 */
	dsdt->checksum = 0;
	dsdt->checksum = table_compute_checksum((void *)dsdt, dsdt->length);

	/*
	 * Fill in platform-specific global NVS variables. If this fails we
	 * cannot return the error but this should only happen while debugging.
	 */
	addr = acpi_create_gnvs(ctx->current);
	if (IS_ERR_VALUE(addr))
		printf("Error: Failed to create GNVS\n");

	acpi_inc_align(ctx, sizeof(struct acpi_global_nvs));

	debug("ACPI:    * FADT\n");
	fadt = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_fadt));
	acpi_create_fadt(fadt, facs, dsdt);
	acpi_add_table(ctx, fadt);

	debug("ACPI:     * SSDT\n");
	ssdt = (struct acpi_table_header *)ctx->current;
	if (!acpi_create_ssdt(ctx, ssdt, OEM_TABLE_ID))
		acpi_add_table(ctx, ssdt);

	debug("ACPI:    * MCFG\n");
	mcfg = ctx->current;
	acpi_create_mcfg(mcfg);
	acpi_inc_align(ctx, mcfg->header.length);
	acpi_add_table(ctx, mcfg);

	if (IS_ENABLED(CONFIG_TPM_V2)) {
		struct acpi_tpm2 *tpm2;

		debug("ACPI:    * TPM2\n");
		tpm2 = (struct acpi_tpm2 *)ctx->current;
		ret = acpi_create_tpm2(tpm2);
		if (!ret) {
			acpi_inc_align(ctx, tpm2->header.length);
			acpi_add_table(ctx, tpm2);
		} else {
			log_warning("TPM2 table creation failed\n");
		}
	}

	debug("ACPI:    * MADT\n");
	madt = ctx->current;
	acpi_create_madt(madt);
	acpi_inc_align(ctx, madt->header.length);
	acpi_add_table(ctx, madt);

	if (IS_ENABLED(CONFIG_TPM_V1)) {
		debug("ACPI:    * TCPA\n");
		tcpa = (struct acpi_tcpa *)ctx->current;
		ret = acpi_create_tcpa(tcpa);
		if (ret) {
			log_warning("Failed to create TCPA table (err=%d)\n",
				    ret);
		} else {
			acpi_inc_align(ctx, tcpa->header.length);
			acpi_add_table(ctx, tcpa);
		}
	}

	debug("ACPI:    * CSRT\n");
	csrt = ctx->current;
	if (!acpi_create_csrt(csrt)) {
		acpi_inc_align(ctx, csrt->header.length);
		acpi_add_table(ctx, csrt);
	}

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

/**
 * acpi_write_hpet() - Write out a HPET table
 *
 * Write out the table for High-Precision Event Timers
 *
 * @hpet: Place to put HPET table
 */
static int acpi_create_hpet(struct acpi_hpet *hpet)
{
	struct acpi_table_header *header = &hpet->header;
	struct acpi_gen_regaddr *addr = &hpet->addr;

	/*
	 * See IA-PC HPET (High Precision Event Timers) Specification v1.0a
	 * https://www.intel.com/content/dam/www/public/us/en/documents/technical-specifications/software-developers-hpet-spec-1-0a.pdf
	 */
	memset((void *)hpet, '\0', sizeof(struct acpi_hpet));

	/* Fill out header fields. */
	acpi_fill_header(header, "HPET");

	header->aslc_revision = ASL_REVISION;
	header->length = sizeof(struct acpi_hpet);
	header->revision = acpi_get_table_revision(ACPITAB_HPET);

	/* Fill out HPET address */
	addr->space_id = 0;  /* Memory */
	addr->bit_width = 64;
	addr->bit_offset = 0;
	addr->addrl = CONFIG_HPET_ADDRESS & 0xffffffff;
	addr->addrh = ((unsigned long long)CONFIG_HPET_ADDRESS) >> 32;

	hpet->id = *(u32 *)CONFIG_HPET_ADDRESS;
	hpet->number = 0;
	hpet->min_tick = 0; /* HPET_MIN_TICKS */

	header->checksum = table_compute_checksum(hpet,
						  sizeof(struct acpi_hpet));

	return 0;
}

int acpi_write_hpet(struct acpi_ctx *ctx)
{
	struct acpi_hpet *hpet;
	int ret;

	log_debug("ACPI:    * HPET\n");

	hpet = ctx->current;
	acpi_inc_align(ctx, sizeof(struct acpi_hpet));
	acpi_create_hpet(hpet);
	ret = acpi_add_table(ctx, hpet);
	if (ret)
		return log_msg_ret("add", ret);

	return 0;
}

int acpi_write_dbg2_pci_uart(struct acpi_ctx *ctx, struct udevice *dev,
			     uint access_size)
{
	struct acpi_dbg2_header *dbg2 = ctx->current;
	char path[ACPI_PATH_MAX];
	struct acpi_gen_regaddr address;
	phys_addr_t addr;
	int ret;

	if (!device_active(dev)) {
		log_info("Device not enabled\n");
		return -EACCES;
	}
	/*
	 * PCI devices don't remember their resource allocation information in
	 * U-Boot at present. We assume that MMIO is used for the UART and that
	 * the address space is 32 bytes: ns16550 uses 8 registers of up to
	 * 32-bits each. This is only for debugging so it is not a big deal.
	 */
	addr = dm_pci_read_bar32(dev, 0);
	log_debug("UART addr %lx\n", (ulong)addr);

	memset(&address, '\0', sizeof(address));
	address.space_id = ACPI_ADDRESS_SPACE_MEMORY;
	address.addrl = (uint32_t)addr;
	address.addrh = (uint32_t)((addr >> 32) & 0xffffffff);
	address.access_size = access_size;

	ret = acpi_device_path(dev, path, sizeof(path));
	if (ret)
		return log_msg_ret("path", ret);
	acpi_create_dbg2(dbg2, ACPI_DBG2_SERIAL_PORT,
			 ACPI_DBG2_16550_COMPATIBLE, &address, 0x1000, path);

	acpi_inc_align(ctx, dbg2->header.length);
	acpi_add_table(ctx, dbg2);

	return 0;
}

void acpi_fadt_common(struct acpi_fadt *fadt, struct acpi_facs *facs,
		      void *dsdt)
{
	struct acpi_table_header *header = &fadt->header;

	memset((void *)fadt, '\0', sizeof(struct acpi_fadt));

	acpi_fill_header(header, "FACP");
	header->length = sizeof(struct acpi_fadt);
	header->revision = 4;
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	memcpy(header->aslc_id, ASLC_ID, 4);
	header->aslc_revision = 1;

	fadt->firmware_ctrl = (unsigned long)facs;
	fadt->dsdt = (unsigned long)dsdt;

	fadt->x_firmware_ctl_l = (unsigned long)facs;
	fadt->x_firmware_ctl_h = 0;
	fadt->x_dsdt_l = (unsigned long)dsdt;
	fadt->x_dsdt_h = 0;

	fadt->preferred_pm_profile = ACPI_PM_MOBILE;

	/* Use ACPI 3.0 revision */
	fadt->header.revision = 4;
}

void acpi_create_dmar_drhd(struct acpi_ctx *ctx, uint flags, uint segment,
			   u64 bar)
{
	struct dmar_entry *drhd = ctx->current;

	memset(drhd, '\0', sizeof(*drhd));
	drhd->type = DMAR_DRHD;
	drhd->length = sizeof(*drhd); /* will be fixed up later */
	drhd->flags = flags;
	drhd->segment = segment;
	drhd->bar = bar;
	acpi_inc(ctx, drhd->length);
}

void acpi_create_dmar_rmrr(struct acpi_ctx *ctx, uint segment, u64 bar,
			   u64 limit)
{
	struct dmar_rmrr_entry *rmrr = ctx->current;

	memset(rmrr, '\0', sizeof(*rmrr));
	rmrr->type = DMAR_RMRR;
	rmrr->length = sizeof(*rmrr); /* will be fixed up later */
	rmrr->segment = segment;
	rmrr->bar = bar;
	rmrr->limit = limit;
	acpi_inc(ctx, rmrr->length);
}

void acpi_dmar_drhd_fixup(struct acpi_ctx *ctx, void *base)
{
	struct dmar_entry *drhd = base;

	drhd->length = ctx->current - base;
}

void acpi_dmar_rmrr_fixup(struct acpi_ctx *ctx, void *base)
{
	struct dmar_rmrr_entry *rmrr = base;

	rmrr->length = ctx->current - base;
}

static int acpi_create_dmar_ds(struct acpi_ctx *ctx, enum dev_scope_type type,
			       uint enumeration_id, pci_dev_t bdf)
{
	/* we don't support longer paths yet */
	const size_t dev_scope_length = sizeof(struct dev_scope) + 2;
	struct dev_scope *ds = ctx->current;

	memset(ds, '\0', dev_scope_length);
	ds->type = type;
	ds->length = dev_scope_length;
	ds->enumeration = enumeration_id;
	ds->start_bus = PCI_BUS(bdf);
	ds->path[0].dev = PCI_DEV(bdf);
	ds->path[0].fn = PCI_FUNC(bdf);

	return ds->length;
}

int acpi_create_dmar_ds_pci_br(struct acpi_ctx *ctx, pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_PCI_SUB, 0, bdf);
}

int acpi_create_dmar_ds_pci(struct acpi_ctx *ctx, pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_PCI_ENDPOINT, 0, bdf);
}

int acpi_create_dmar_ds_ioapic(struct acpi_ctx *ctx, uint enumeration_id,
			       pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_IOAPIC, enumeration_id, bdf);
}

int acpi_create_dmar_ds_msi_hpet(struct acpi_ctx *ctx, uint enumeration_id,
				 pci_dev_t bdf)
{
	return acpi_create_dmar_ds(ctx, SCOPE_MSI_HPET, enumeration_id, bdf);
}

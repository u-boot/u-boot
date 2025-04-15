// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic code used to generate ACPI tables
 *
 * Copyright 2019 Google LLC
 */

#include <bloblist.h>
#include <cpu.h>
#include <dm.h>
#include <efi_api.h>
#include <efi_loader.h>
#include <log.h>
#include <mapmem.h>
#include <tables_csum.h>
#include <serial.h>
#include <version_string.h>
#include <acpi/acpi_table.h>
#include <acpi/acpi_device.h>
#include <asm/global_data.h>
#include <dm/acpi.h>
#include <linux/sizes.h>
#include <linux/log2.h>

enum {
	TABLE_SIZE	= SZ_64K,
};

DECLARE_GLOBAL_DATA_PTR;

/*
 * OEM_REVISION is 32-bit unsigned number. It should be increased only when
 * changing software version. Therefore it should not depend on build time.
 * U-Boot calculates it from U-Boot version and represent it in hexadecimal
 * notation. As U-Boot version is in form year.month set low 8 bits to 0x01
 * to have valid date. So for U-Boot version 2021.04 OEM_REVISION is set to
 * value 0x20210401.
 */
#define OEM_REVISION ((((version_num / 1000) % 10) << 28) | \
		      (((version_num / 100) % 10) << 24) | \
		      (((version_num / 10) % 10) << 20) | \
		      ((version_num % 10) << 16) | \
		      (((version_num_patch / 10) % 10) << 12) | \
		      ((version_num_patch % 10) << 8) | \
		      0x01)

int acpi_create_dmar(struct acpi_dmar *dmar, enum dmar_flags flags)
{
	struct acpi_table_header *header = &dmar->header;
	struct cpu_info info;
	struct udevice *cpu;
	int ret;

	ret = uclass_first_device_err(UCLASS_CPU, &cpu);
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
	header->checksum = table_compute_checksum(dmar, header->length);

	return 0;
}

int acpi_get_table_revision(enum acpi_tables table)
{
	switch (table) {
	case ACPITAB_FADT:
		return ACPI_FADT_REV_ACPI_6_0;
	case ACPITAB_MADT:
		return ACPI_MADT_REV_ACPI_6_2;
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
	case ACPITAB_PPTT: /* ACPI 6.2: 1 */
		return 1;
	case ACPITAB_GTDT: /* ACPI 6.2: 2, ACPI 6.3: 3 */
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
	header->oem_revision = OEM_REVISION;
	memcpy(header->creator_id, ASLC_ID, 4);
	header->creator_revision = ASL_REVISION;
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

	/* On legacy x86 platforms the RSDT is mandatory while the XSDT is not.
	 * On other platforms there might be no memory below 4GiB, thus RSDT is NULL.
	 */
	if (ctx->rsdt) {
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
		rsdt->entry[i] = nomap_to_sysmem(table);

		/* Fix RSDT length or the kernel will assume invalid entries */
		rsdt->header.length = sizeof(struct acpi_table_header) +
					(sizeof(u32) * (i + 1));

		/* Re-calculate checksum */
		acpi_update_checksum(&rsdt->header);
	}

	if (ctx->xsdt) {
		/*
		 * And now the same thing for the XSDT. We use the same index as for
		 * now we want the XSDT and RSDT to always be in sync in U-Boot
		 */
		xsdt = ctx->xsdt;

		/* This should always be MAX_ACPI_TABLES */
		entries_num = ARRAY_SIZE(xsdt->entry);

		for (i = 0; i < entries_num; i++) {
			if (xsdt->entry[i] == 0)
				break;
		}

		if (i >= entries_num) {
			log_err("ACPI: Error: too many tables\n");
			return -E2BIG;
		}

		/* Add table to the XSDT */
		xsdt->entry[i] = nomap_to_sysmem(table);

		/* Fix XSDT length */
		xsdt->header.length = sizeof(struct acpi_table_header) +
					(sizeof(u64) * (i + 1));

		/* Re-calculate checksum */
		acpi_update_checksum(&xsdt->header);
	}

	return 0;
}

int acpi_write_fadt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_fadt *fadt;

	fadt = ctx->current;
	header = &fadt->header;

	memset((void *)fadt, '\0', sizeof(struct acpi_fadt));

	acpi_fill_header(header, "FACP");
	header->length = sizeof(struct acpi_fadt);
	header->revision = acpi_get_table_revision(ACPITAB_FADT);
	memcpy(header->oem_id, OEM_ID, 6);
	memcpy(header->oem_table_id, OEM_TABLE_ID, 8);
	memcpy(header->creator_id, ASLC_ID, 4);
	header->creator_revision = 1;
	fadt->minor_revision = 2;

	fadt->x_firmware_ctrl = nomap_to_sysmem(ctx->facs);
	fadt->x_dsdt = nomap_to_sysmem(ctx->dsdt);

	if (fadt->x_firmware_ctrl < 0x100000000ULL)
		fadt->firmware_ctrl = fadt->x_firmware_ctrl;

	if (fadt->x_dsdt < 0x100000000ULL)
		fadt->dsdt = fadt->x_dsdt;

	fadt->preferred_pm_profile = ACPI_PM_UNSPECIFIED;

	acpi_fill_fadt(fadt);

	acpi_update_checksum(header);

	return acpi_add_fadt(ctx, fadt);
}

#ifndef CONFIG_QFW_ACPI
ACPI_WRITER(5fadt, "FADT", acpi_write_fadt, 0);
#endif

int acpi_write_madt(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_header *header;
	struct acpi_madt *madt;
	void *current;

	madt = ctx->current;

	memset(madt, '\0', sizeof(struct acpi_madt));
	header = &madt->header;

	/* Fill out header fields */
	acpi_fill_header(header, "APIC");
	header->length = sizeof(struct acpi_madt);
	header->revision = acpi_get_table_revision(ACPITAB_MADT);

	acpi_inc(ctx, sizeof(struct acpi_madt));
	/* TODO: Get rid of acpi_fill_madt and use driver model */
	current = acpi_fill_madt(madt, ctx);

	/* (Re)calculate length and checksum */
	header->length = (uintptr_t)current - (uintptr_t)madt;

	if (IS_ENABLED(CONFIG_ACPI_PARKING_PROTOCOL))
		acpi_write_park(madt);

	acpi_update_checksum(header);
	acpi_add_table(ctx, madt);
	ctx->current = (void *)madt + madt->header.length;

	return 0;
}

#ifndef CONFIG_QFW_ACPI
ACPI_WRITER(5madt, "MADT", acpi_write_madt, 0);
#endif

void acpi_create_dbg2(struct acpi_dbg2_header *dbg2,
		      int port_type, int port_subtype,
		      struct acpi_gen_regaddr *address, u32 address_size,
		      const char *device_path)
{
	uintptr_t current;
	struct acpi_dbg2_device *device;
	u32 *dbg2_addr_size;
	struct acpi_table_header *header;
	size_t path_len;
	const char *path;
	char *namespace;

	/* Fill out header fields. */
	current = (uintptr_t)dbg2;
	memset(dbg2, '\0', sizeof(struct acpi_dbg2_header));
	header = &dbg2->header;

	header->revision = acpi_get_table_revision(ACPITAB_DBG2);
	acpi_fill_header(header, "DBG2");

	/* One debug device defined */
	dbg2->devices_offset = sizeof(struct acpi_dbg2_header);
	dbg2->devices_count = 1;
	current += sizeof(struct acpi_dbg2_header);

	/* Device comes after the header */
	device = (struct acpi_dbg2_device *)current;
	memset(device, 0, sizeof(struct acpi_dbg2_device));
	current += sizeof(struct acpi_dbg2_device);

	device->revision = 0;
	device->address_count = 1;
	device->port_type = port_type;
	device->port_subtype = port_subtype;

	/* Base Address comes after device structure */
	memcpy((void *)current, address, sizeof(struct acpi_gen_regaddr));
	device->base_address_offset = current - (uintptr_t)device;
	current += sizeof(struct acpi_gen_regaddr);

	/* Address Size comes after address structure */
	dbg2_addr_size = (uint32_t *)current;
	device->address_size_offset = current - (uintptr_t)device;
	*dbg2_addr_size = address_size;
	current += sizeof(uint32_t);

	/* Namespace string comes last, use '.' if not provided */
	path = device_path ? : ".";
	/* Namespace string length includes NULL terminator */
	path_len = strlen(path) + 1;
	namespace = (char *)current;
	device->namespace_string_length = path_len;
	device->namespace_string_offset = current - (uintptr_t)device;
	strncpy(namespace, path, path_len);
	current += path_len;

	/* Update structure lengths and checksum */
	device->length = current - (uintptr_t)device;
	header->length = current - (uintptr_t)dbg2;
	acpi_update_checksum(header);
}

int acpi_write_dbg2_pci_uart(struct acpi_ctx *ctx, struct udevice *dev,
			     uint access_size)
{
	struct acpi_dbg2_header *dbg2 = ctx->current;
	char path[ACPI_PATH_MAX];
	struct acpi_gen_regaddr address;
	u64 addr;
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

	ret = acpi_device_path(dev, path, sizeof(path));
	if (ret)
		return log_msg_ret("path", ret);

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

static int acpi_write_spcr(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct serial_device_info serial_info = {0};
	u64 serial_address, serial_offset;
	struct acpi_table_header *header;
	struct acpi_spcr *spcr;
	struct udevice *dev;
	uint serial_config;
	uint serial_width;
	int access_size;
	int space_id;
	int ret = -ENODEV;

	spcr = ctx->current;
	header = &spcr->header;

	memset(spcr, '\0', sizeof(struct acpi_spcr));

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
	case SERIAL_CHIP_PL01X:
		spcr->interface_type = ACPI_DBG2_ARM_PL011;
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
	serial_offset = ((u64)serial_info.reg_offset) << serial_info.reg_shift;
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

	debug("UART type %u @ %llx\n", spcr->interface_type, serial_address);

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
	acpi_update_checksum(header);

	acpi_add_table(ctx, spcr);
	acpi_inc(ctx, spcr->header.length);

	return 0;
}

ACPI_WRITER(5spcr, "SPCR", acpi_write_spcr, 0);

__weak int acpi_fill_iort(struct acpi_ctx *ctx)
{
	return 0;
}

int acpi_iort_add_its_group(struct acpi_ctx *ctx,
			    const u32 its_count,
			    const u32 *identifiers)
{
	struct acpi_iort_node *node;
	struct acpi_iort_its_group *group;
	int offset;

	offset = ctx->current - ctx->tab_start;

	node = ctx->current;
	memset(node, '\0', sizeof(struct acpi_iort_node));

	node->type = ACPI_IORT_NODE_ITS_GROUP;
	node->revision = 1;

	node->length = sizeof(struct acpi_iort_node);
	node->length += sizeof(struct acpi_iort_its_group);
	node->length += sizeof(u32) * its_count;

	group = (struct acpi_iort_its_group *)node->node_data;
	group->its_count = its_count;
	memcpy(&group->identifiers, identifiers, sizeof(u32) * its_count);

	ctx->current += node->length;

	return offset;
}

int acpi_iort_add_named_component(struct acpi_ctx *ctx,
				  const u32 node_flags,
				  const u64 memory_properties,
				  const u8 memory_address_limit,
				  const char *device_name)
{
	struct acpi_iort_node *node;
	struct acpi_iort_named_component *comp;
	int offset;

	offset = ctx->current - ctx->tab_start;

	node = ctx->current;
	memset(node, '\0', sizeof(struct acpi_iort_node));

	node->type = ACPI_IORT_NODE_NAMED_COMPONENT;
	node->revision = 4;
	node->length = sizeof(struct acpi_iort_node);
	node->length += sizeof(struct acpi_iort_named_component);
	node->length += strlen(device_name) + 1;

	comp = (struct acpi_iort_named_component *)node->node_data;
	memset(comp, '\0', sizeof(struct acpi_iort_named_component));

	comp->node_flags = node_flags;
	comp->memory_properties = memory_properties;
	comp->memory_address_limit = memory_address_limit;
	memcpy(comp->device_name, device_name, strlen(device_name) + 1);

	ctx->current += node->length;

	return offset;
}

int acpi_iort_add_rc(struct acpi_ctx *ctx,
		     const u64 mem_access_properties,
		     const u32 ats_attributes,
		     const u32 pci_segment_number,
		     const u8 memory_address_size_limit,
		     const int num_mappings,
		     const struct acpi_iort_id_mapping *map)
{
	struct acpi_iort_id_mapping *mapping;
	struct acpi_iort_node *output_node;
	struct acpi_iort_node *node;
	struct acpi_iort_rc *rc;
	int offset;

	offset = ctx->current - ctx->tab_start;

	node = ctx->current;
	memset(node, '\0', sizeof(struct acpi_iort_node));

	node->type = ACPI_IORT_NODE_PCI_ROOT_COMPLEX;
	node->revision = 2;
	node->mapping_count = num_mappings;
	if (num_mappings)
		node->mapping_offset = sizeof(struct acpi_iort_node) +
				       sizeof(struct acpi_iort_rc);

	node->length = sizeof(struct acpi_iort_node);
	node->length += sizeof(struct acpi_iort_rc);
	node->length += sizeof(struct acpi_iort_id_mapping) * num_mappings;

	rc = (struct acpi_iort_rc *)node->node_data;
	memset(rc, '\0', sizeof(struct acpi_iort_rc));

	rc->mem_access_properties = mem_access_properties;
	rc->ats_attributes = ats_attributes;
	rc->pci_segment_number = pci_segment_number;
	rc->memory_address_size_limit = memory_address_size_limit;

	mapping = (struct acpi_iort_id_mapping *)(rc + 1);
	for (int i = 0; i < num_mappings; i++) {
		/* Validate input */
		output_node = (struct acpi_iort_node *)ctx->tab_start + map[i].output_reference;
		/* ID mappings can use SMMUs or ITS groups as output references */
		assert(output_node && ((output_node->type == ACPI_IORT_NODE_ITS_GROUP) ||
				       (output_node->type == ACPI_IORT_NODE_SMMU) ||
				       (output_node->type == ACPI_IORT_NODE_SMMU_V3)));

		memcpy(mapping, &map[i], sizeof(struct acpi_iort_id_mapping));
		mapping++;
	}

	ctx->current += node->length;

	return offset;
}

int acpi_iort_add_smmu_v3(struct acpi_ctx *ctx,
			  const u64 base_address,
			  const u32 flags,
			  const u64 vatos_address,
			  const u32 model,
			  const u32 event_gsiv,
			  const u32 pri_gsiv,
			  const u32 gerr_gsiv,
			  const u32 sync_gsiv,
			  const u32 pxm,
			  const u32 id_mapping_index,
			  const int num_mappings,
			  const struct acpi_iort_id_mapping *map)
{
	struct acpi_iort_node *node;
	struct acpi_iort_node *output_node;
	struct acpi_iort_smmu_v3 *smmu;
	struct acpi_iort_id_mapping *mapping;
	int offset;

	offset = ctx->current - ctx->tab_start;

	node = ctx->current;
	memset(node, '\0', sizeof(struct acpi_iort_node));

	node->type = ACPI_IORT_NODE_SMMU_V3;
	node->revision = 5;
	node->mapping_count = num_mappings;
	if (num_mappings)
		node->mapping_offset = sizeof(struct acpi_iort_node) +
				       sizeof(struct acpi_iort_smmu_v3);

	node->length = sizeof(struct acpi_iort_node);
	node->length += sizeof(struct acpi_iort_smmu_v3);
	node->length += sizeof(struct acpi_iort_id_mapping) * num_mappings;

	smmu = (struct acpi_iort_smmu_v3 *)node->node_data;
	memset(smmu, '\0', sizeof(struct acpi_iort_smmu_v3));

	smmu->base_address = base_address;
	smmu->flags = flags;
	smmu->vatos_address = vatos_address;
	smmu->model = model;
	smmu->event_gsiv = event_gsiv;
	smmu->pri_gsiv = pri_gsiv;
	smmu->gerr_gsiv = gerr_gsiv;
	smmu->sync_gsiv = sync_gsiv;
	smmu->pxm = pxm;
	smmu->id_mapping_index = id_mapping_index;

	mapping = (struct acpi_iort_id_mapping *)(smmu + 1);
	for (int i = 0; i < num_mappings; i++) {
		/* Validate input */
		output_node = (struct acpi_iort_node *)ctx->tab_start + map[i].output_reference;
		/*
		 * ID mappings of an SMMUv3 node can only have ITS group nodes
		 * as output references.
		 */
		assert(output_node && output_node->type == ACPI_IORT_NODE_ITS_GROUP);

		memcpy(mapping, &map[i], sizeof(struct acpi_iort_id_mapping));
		mapping++;
	}

	ctx->current += node->length;

	return offset;
}

static int acpi_write_iort(struct acpi_ctx *ctx, const struct acpi_writer *entry)
{
	struct acpi_table_iort *iort;
	struct acpi_iort_node *node;
	u32 offset;
	int ret;

	iort = ctx->current;
	ctx->tab_start = ctx->current;
	memset(iort, '\0', sizeof(struct acpi_table_iort));

	acpi_fill_header(&iort->header, "IORT");
	iort->header.revision = 1;
	iort->header.creator_revision = 1;
	iort->header.length = sizeof(struct acpi_table_iort);
	iort->node_offset = sizeof(struct acpi_table_iort);

	acpi_inc(ctx, sizeof(struct acpi_table_iort));

	offset = sizeof(struct acpi_table_iort);
	ret = acpi_fill_iort(ctx);
	if (ret) {
		ctx->current = iort;
		return log_msg_ret("fill", ret);
	}

	/* Count nodes filled in */
	for (node = (void *)iort + iort->node_offset;
	     node->length > 0 && (void *)node < ctx->current;
	     node = (void *)node + node->length)
		iort->node_count++;

	/* (Re)calculate length and checksum */
	iort->header.length = ctx->current - (void *)iort;
	acpi_update_checksum(&iort->header);
	log_debug("IORT at %p, length %x\n", iort, iort->header.length);

	/* Drop the table if it is empty */
	if (iort->header.length == sizeof(struct acpi_table_iort))
		return log_msg_ret("fill", -ENOENT);
	acpi_add_table(ctx, iort);

	return 0;
}

ACPI_WRITER(5iort, "IORT", acpi_write_iort, 0);

/*
 * Allocate memory for ACPI tables and write ACPI tables to the
 * allocated buffer.
 *
 * Return:	status code
 */
static int alloc_write_acpi_tables(void)
{
	u64 table_end;
	void *addr;

	if (IS_ENABLED(CONFIG_X86) ||
	    IS_ENABLED(CONFIG_QFW_ACPI) ||
	    IS_ENABLED(CONFIG_SANDBOX)) {
		log_debug("Skipping writing ACPI tables as already done\n");
		return 0;
	}

	if (!IS_ENABLED(CONFIG_BLOBLIST_TABLES)) {
		log_debug("Skipping writing ACPI tables as BLOBLIST_TABLES is not selected\n");
		return 0;
	}

	/* Align the table to a 4KB boundary to keep EFI happy */
	addr = bloblist_add(BLOBLISTT_ACPI_TABLES, TABLE_SIZE,
			    ilog2(SZ_4K));

	if (!addr)
		return log_msg_ret("mem", -ENOMEM);

	gd->arch.table_start_high = virt_to_phys(addr);
	gd->arch.table_end_high = gd->arch.table_start_high + TABLE_SIZE;

	table_end = write_acpi_tables(gd->arch.table_start_high);
	if (!table_end) {
		log_err("Can't create ACPI configuration table\n");
		return -EINTR;
	}

	log_debug("- wrote 'acpi' to %lx, end %llx\n", gd->arch.table_start_high, table_end);
	if (table_end > gd->arch.table_end_high) {
		log_err("Out of space for configuration tables: need %llx, have %x\n",
			table_end - gd->arch.table_start_high, TABLE_SIZE);
		return log_msg_ret("acpi", -ENOSPC);
	}

	log_debug("- done writing tables\n");

	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, alloc_write_acpi_tables);

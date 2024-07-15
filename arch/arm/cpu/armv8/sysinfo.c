// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <dm.h>
#if CONFIG_IS_ENABLED(SYSINFO_SMBIOS)
#include <smbios_plat.h>
#endif
#include <stdio.h>
#include <sysinfo.h>

#define ELEMENT_RECORD_DATA_LEN 8
#define ELEMENT_RECORD_NUM 2

union midr_el1 {
	struct {
		u64 revision:4;
		u64 partnum:12;
		u64 architecture:4;
		u64 variant:4;
		u64 implementer:8;
		u64 reserved:32;
	} fields;
	u64 data;
};

enum {
	VENDOR_RESERVED = 0,
	VENDOR_ARM = 0x41,
	VENDOR_BROADCOM = 0x42,
	VENDOR_CAVIUM = 0x43,
	VENDOR_DEC = 0x44,
	VENDOR_FUJITSU = 0x46,
	VENDOR_INFINEON = 0x49,
	VENDOR_FREESCALE = 0x4d,
	VENDOR_NVIDIA = 0x4e,
	VENDOR_AMCC = 0x50,
	VENDOR_QUALCOMM = 0x51,
	VENDOR_MARVELL = 0x56,
	VENDOR_INTEL = 0x69,
	VENDOR_AMPERE = 0xc0,
};

struct __packed enclosure_element_record {
	struct elem_hdr hdr;
	u8 data[ELEMENT_RECORD_DATA_LEN];
};

struct processor_info processor_info_armv8;

/* Default data for the enclosure contained elements */
struct enclosure_element_record contained_elements[ELEMENT_RECORD_NUM] = {
	{
		.hdr.type = SMBIOS_BOARD_MOTHERBOARD,
		.hdr.minimum = 0,
		.hdr.maximum = 0xff,
		.data = {0},
	},
	{
		.hdr.type = SMBIOS_BOARD_MOTHERBOARD,
		.hdr.minimum = 0,
		.hdr.maximum = 0xff,
		.data = {0},
	},
};

int sysinfo_get_processor_info(struct processor_info *pinfo)
{
	u64 mpidr, core_count;
	union midr_el1 midr;

	/* Read the MIDR_EL1 register */
	asm volatile("mrs %0, MIDR_EL1" : "=r"(midr.data));
	/* Read the MPIDR_EL1 register */
	asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));

	log_debug("MIDR: 0x%016llx\n", midr.data);
	log_debug("MPIDR: 0x%016llx\n", mpidr);
	log_debug("CPU Implementer: 0x%02x\n", midr.fields.implementer);

	switch (midr.fields.implementer) {
	case VENDOR_ARM:
		pinfo->manufacturer = "ARM Limited";
		break;
	case VENDOR_BROADCOM:
		pinfo->manufacturer = "Broadcom Corporation";
		break;
	case VENDOR_CAVIUM:
		pinfo->manufacturer = "Cavium Inc";
		break;
	case VENDOR_DEC:
		pinfo->manufacturer = "Digital Equipment Corporation";
		break;
	case VENDOR_FUJITSU:
		pinfo->manufacturer = "Fujitsu Ltd";
		break;
	case VENDOR_INFINEON:
		pinfo->manufacturer = "Infineon Technologies AG";
		break;
	case VENDOR_FREESCALE:
		pinfo->manufacturer = "Freescale Semiconductor Inc";
		break;
	case VENDOR_NVIDIA:
		pinfo->manufacturer = "NVIDIA Corporation";
		break;
	case VENDOR_AMCC:
		pinfo->manufacturer =
			"Applied Micro Circuits Corporation";
		break;
	case VENDOR_QUALCOMM:
		pinfo->manufacturer = "Qualcomm Inc";
		break;
	case VENDOR_MARVELL:
		pinfo->manufacturer = "Marvell International Ltd";
		break;
	case VENDOR_INTEL:
		pinfo->manufacturer = "Intel Corporation";
		break;
	case VENDOR_AMPERE:
		pinfo->manufacturer = "Ampere Computing";
		break;
	default:
		pinfo->manufacturer = "Unknown";
		break;
	}
	log_debug("CPU part number: 0x%x\n", midr.fields.partnum);
	log_debug("CPU revision: 0x%x\n", midr.fields.revision);
	log_debug("CPU architecture: 0x%x\n", midr.fields.architecture);
	log_debug("CPU variant: 0x%x\n", midr.fields.variant);

	pinfo->version = "Not Specified";

	/* Extract number of cores */
	core_count = (mpidr >> 0) & 0xFF;
	pinfo->core_count = core_count + 1;
	log_debug("CPU Core Count: %d\n", pinfo->core_count);

	/* Other default processor information */
	pinfo->type = SMBIOS_PROCESSOR_TYPE_CENTRAL;
	pinfo->status = SMBIOS_PROCESSOR_STATUS_ENABLED;
	pinfo->upgrade = SMBIOS_PROCESSOR_UPGRADE_NONE;
	pinfo->family2 = SMBIOS_PROCESSOR_FAMILY_ARMV8;
	pinfo->socket_design = "Not Specified";
	pinfo->sn = "Not Specified";
	pinfo->asset_tag = "Not Specified";
	pinfo->pn = "Not Specified";
	pinfo->core_enabled = pinfo->core_count;
	pinfo->characteristics = SMBIOS_PROCESSOR_64BIT |
		SMBIOS_PROCESSOR_ARM64_SOCID;
	if (pinfo->core_count > 1)
		pinfo->characteristics |= SMBIOS_PROCESSOR_MULTICORE;

	return 0;
}

struct sysinfo_plat sysinfo_smbios_armv8 = {
	/* System Information */
	.sys.manufacturer = "arm",
	.sys.prod_name = "arm",
	.sys.version = "armv8",
	.sys.sn = "Not Specified",
	.sys.wakeup_type = SMBIOS_WAKEUP_TYPE_UNKNOWN,
	.sys.sku_num = "Not Specified",
	.sys.family = "arm",

	/* Baseboard (or Module) Information */
	.board.manufacturer = "arm",
	.board.prod_name = "arm",
	.board.version = "armv8",
	.board.sn = "Not Specified",
	.board.asset_tag = "Not Specified",
	.board.feature.fields.hosting_board = 1, /* host board */
	.board.chassis_locat = "Not Specified",
	.board.type = SMBIOS_BOARD_MOTHERBOARD,

	/* System Enclosure or Chassis Information */
	.chassis.manufacturer = "arm",
	.chassis.version = "armv8",
	.chassis.sn = "Not Specified",
	.chassis.asset_tag = "Not Specified",
	.chassis.chassis_type = SMBIOS_ENCLOSURE_DESKTOP,
	.chassis.bootup_state = SMBIOS_STATE_SAFE,
	.chassis.power_supply_state = SMBIOS_STATE_SAFE,
	.chassis.thermal_state = SMBIOS_STATE_SAFE,
	.chassis.security_status = SMBIOS_SECURITY_NONE,
	.chassis.oem_defined = SMBIOS_ENCLOSURE_OEM_UND,
	.chassis.height = SMBIOS_ENCLOSURE_HEIGHT_UND,
	.chassis.number_of_power_cords = SMBIOS_POWCORD_NUM_UND,
	.chassis.element_count = ARRAY_SIZE(contained_elements),
	.chassis.element_record_length = sizeof(contained_elements[0]),
	.chassis.elements = &contained_elements[0],
	.chassis.elements_size = sizeof(contained_elements),
	.chassis.sku_num = "Not Specified",

	/* Processor Information */
	.processor = &processor_info_armv8,
};

U_BOOT_DRVINFO(sysinfo_smbios_plat) = {
	.name = "sysinfo_smbios_plat",
	.plat = &sysinfo_smbios_armv8,
};

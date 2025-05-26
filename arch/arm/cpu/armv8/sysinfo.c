// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2024 Linaro Limited
 * Author: Raymond Mao <raymond.mao@linaro.org>
 */
#include <dm.h>
#include <smbios_plat.h>
#include <stdio.h>
#include <sysinfo.h>

union ccsidr_el1 {
	struct {
		u64 linesize:3;
		u64 associativity:10;
		u64 numsets:15;
		u64 unknown:4;
		u64 reserved:32;
	} no_ccidx;
	struct {
		u64 linesize:3;
		u64 associativity:21;
		u64 reserved1:8;
		u64 numsets:24;
		u64 reserved2:8;
	} ccidx_aarch64;
	struct {
		u64 linesize:3;
		u64 associativity:21;
		u64 reserved:8;
		u64 unallocated:32;
	} ccidx_aarch32;
	u64 data;
};

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
	CACHE_NONE,
	CACHE_INST_ONLY,
	CACHE_DATA_ONLY,
	CACHE_INST_WITH_DATA,
	CACHE_UNIFIED,
};

enum {
	CACHE_ASSOC_DIRECT_MAPPED = 1,
	CACHE_ASSOC_2WAY = 2,
	CACHE_ASSOC_4WAY = 4,
	CACHE_ASSOC_8WAY = 8,
	CACHE_ASSOC_16WAY = 16,
	CACHE_ASSOC_12WAY = 12,
	CACHE_ASSOC_24WAY = 24,
	CACHE_ASSOC_32WAY = 32,
	CACHE_ASSOC_48WAY = 48,
	CACHE_ASSOC_64WAY = 64,
	CACHE_ASSOC_20WAY = 20,
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

/*
 * TODO:
 * To support ARMv8.3, we need to read "CCIDX, bits [23:20]" from
 * ID_AA64MMFR2_EL1 to get the format of CCSIDR_EL1:
 *
 * 0b0000 - 32-bit format implemented for all levels of the CCSIDR_EL1.
 * 0b0001 - 64-bit format implemented for all levels of the CCSIDR_EL1.
 *
 * Here we assume to use CCSIDR_EL1 in no CCIDX layout:
 * NumSets, bits [27:13]: (Number of sets in cache) - 1
 * Associativity, bits [12:3]: (Associativity of cache) - 1
 * LineSize, bits [2:0]: (Log2(Number of bytes in cache line)) - 4
 */
int sysinfo_get_cache_info(u8 level, struct cache_info *cinfo)
{
	u64 clidr_el1;
	u32 csselr_el1;
	u32 num_sets;
	union ccsidr_el1 creg;
	int cache_type;

	sysinfo_cache_info_default(cinfo);

	/* Read CLIDR_EL1 */
	asm volatile("mrs %0, clidr_el1" : "=r" (clidr_el1));
	debug("CLIDR_EL1: 0x%llx\n", clidr_el1);

	cache_type = (clidr_el1 >> (3 * level)) & 0x7;
	if (cache_type == CACHE_NONE) /* level does not exist */
		return -1;

	switch (cache_type) {
	case CACHE_INST_ONLY:
		cinfo->cache_type = SMBIOS_CACHE_SYSCACHE_TYPE_INST;
		break;
	case CACHE_DATA_ONLY:
		cinfo->cache_type = SMBIOS_CACHE_SYSCACHE_TYPE_DATA;
		break;
	case CACHE_UNIFIED:
		cinfo->cache_type = SMBIOS_CACHE_SYSCACHE_TYPE_UNIFIED;
		break;
	case CACHE_INST_WITH_DATA:
		cinfo->cache_type = SMBIOS_CACHE_SYSCACHE_TYPE_OTHER;
		break;
	default:
		cinfo->cache_type = SMBIOS_CACHE_SYSCACHE_TYPE_UNKNOWN;
		break;
	}

	/* Select cache level */
	csselr_el1 = (level << 1);
	asm volatile("msr csselr_el1, %0" : : "r" ((u64)csselr_el1));

	/* Read CCSIDR_EL1 */
	asm volatile("mrs %0, ccsidr_el1" : "=r" (creg.data));
	debug("CCSIDR_EL1 (Level %d): 0x%llx\n", level + 1, creg.data);

	/* Extract cache size and associativity */
	cinfo->line_size = 1 << (creg.no_ccidx.linesize + 4);

	/* Map the associativity value */
	switch (creg.no_ccidx.associativity + 1) {
	case CACHE_ASSOC_DIRECT_MAPPED:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_DMAPPED;
		break;
	case CACHE_ASSOC_2WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_2WAY;
		break;
	case CACHE_ASSOC_4WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_4WAY;
		break;
	case CACHE_ASSOC_8WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_8WAY;
		break;
	case CACHE_ASSOC_16WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_16WAY;
		break;
	case CACHE_ASSOC_12WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_12WAY;
		break;
	case CACHE_ASSOC_24WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_24WAY;
		break;
	case CACHE_ASSOC_32WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_32WAY;
		break;
	case CACHE_ASSOC_48WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_48WAY;
		break;
	case CACHE_ASSOC_64WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_64WAY;
		break;
	case CACHE_ASSOC_20WAY:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_20WAY;
		break;
	default:
		cinfo->associativity = SMBIOS_CACHE_ASSOC_UNKNOWN;
		break;
	}

	num_sets = creg.no_ccidx.numsets + 1;
	/* Size in KB */
	cinfo->max_size = (cinfo->associativity * num_sets * cinfo->line_size) /
			  1024;

	debug("L%d Cache:\n", level + 1);
	debug("Number of bytes in cache line:%u\n", cinfo->line_size);
	debug("Associativity of cache:%u\n", cinfo->associativity);
	debug("Number of sets in cache:%u\n", num_sets);
	debug("Cache size in KB:%u\n", cinfo->max_size);

	cinfo->inst_size = cinfo->max_size;

	/*
	 * Below fields with common values are placed under DT smbios node
	 * socket-design, config
	 * Other fields are typically specific to the implementation of the ARM
	 * processor by the silicon vendor:
	 * supp_sram_type, curr_sram_type, speed, err_corr_type
	 */

	return 0;
}

int sysinfo_get_processor_info(struct processor_info *pinfo)
{
	u64 mpidr, core_count;
	union midr_el1 midr;

	/* Read the MIDR_EL1 register */
	asm volatile("mrs %0, MIDR_EL1" : "=r"(midr.data));
	/* Read the MPIDR_EL1 register */
	asm volatile("mrs %0, MPIDR_EL1" : "=r"(mpidr));

	debug("MIDR: 0x%016llx\n", midr.data);
	debug("MPIDR: 0x%016llx\n", mpidr);
	debug("CPU Implementer: 0x%02x\n", midr.fields.implementer);

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
	debug("CPU part number: 0x%x\n", midr.fields.partnum);
	debug("CPU revision: 0x%x\n", midr.fields.revision);
	debug("CPU architecture: 0x%x\n", midr.fields.architecture);
	debug("CPU variant: 0x%x\n", midr.fields.variant);

	/* Extract number of cores */
	core_count = (mpidr >> 0) & 0xFF;
	pinfo->core_count = core_count + 1;
	debug("CPU Core Count: %d\n", pinfo->core_count);

	pinfo->core_enabled = pinfo->core_count;
	pinfo->characteristics = SMBIOS_PROCESSOR_64BIT |
		SMBIOS_PROCESSOR_ARM64_SOCID;
	if (pinfo->core_count > 1)
		pinfo->characteristics |= SMBIOS_PROCESSOR_MULTICORE;

	/*
	 * Below fields with common values are placed under DT smbios node
	 * version, processor-type, processor-status, upgrade, family2,
	 * socket-design, serial, asset-tag, part-number
	 */

	return 0;
}

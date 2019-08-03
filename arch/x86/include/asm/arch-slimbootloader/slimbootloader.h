/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 */

#ifndef __SLIMBOOTLOADER_ARCH_H__
#define __SLIMBOOTLOADER_ARCH_H__

#include <common.h>
#include <asm/hob.h>

/**
 * A GUID to get MemoryMap info hob which is provided by Slim Bootloader
 */
#define SBL_MEMORY_MAP_INFO_GUID \
	EFI_GUID(0xa1ff7424, 0x7a1a, 0x478e, \
		0xa9, 0xe4, 0x92, 0xf3, 0x57, 0xd1, 0x28, 0x32)

/**
 * A single entry of memory map information
 *
 * @addr: start address of a memory map entry
 * @size: size of a memory map entry
 * @type: usable:1, reserved:2, acpi:3, nvs:4, unusable:5
 * @flag: only used in Slim Bootloader
 * @rsvd: padding for alignment
 */
struct sbl_memory_map_entry {
	u64	addr;
	u64	size;
	u8	type;
	u8	flag;
	u8	rsvd[6];
};

/**
 * This includes all memory map entries which is sorted based on physical start
 * address, from low to high, and carved out reserved, acpi nvs, acpi reclaim
 * and usable memory.
 *
 * @rev  : revision of memory_map_info structure. currently 1.
 * @rsvd : padding for alignment
 * @count: the number of memory map entries
 * @entry: array of all memory map entries
 */
struct sbl_memory_map_info {
	u8	rev;
	u8	rsvd[3];
	u32	count;
	struct sbl_memory_map_entry	entry[0];
};

#endif /* __SLIMBOOTLOADER_ARCH_H__ */

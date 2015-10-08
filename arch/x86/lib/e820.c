/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/e820.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Install a default e820 table with 4 entries as follows:
 *
 *	0x000000-0x0a0000	Useable RAM
 *	0x0a0000-0x100000	Reserved for ISA
 *	0x100000-gd->ram_size	Useable RAM
 *	CONFIG_PCIE_ECAM_BASE	PCIe ECAM
 */
__weak unsigned install_e820_map(unsigned max_entries,
				 struct e820entry *entries)
{
	entries[0].addr = 0;
	entries[0].size = ISA_START_ADDRESS;
	entries[0].type = E820_RAM;
	entries[1].addr = ISA_START_ADDRESS;
	entries[1].size = ISA_END_ADDRESS - ISA_START_ADDRESS;
	entries[1].type = E820_RESERVED;
	entries[2].addr = ISA_END_ADDRESS;
	entries[2].size = gd->ram_size - ISA_END_ADDRESS;
	entries[2].type = E820_RAM;
	entries[3].addr = CONFIG_PCIE_ECAM_BASE;
	entries[3].size = CONFIG_PCIE_ECAM_SIZE;
	entries[3].type = E820_RESERVED;

	return 4;
}

/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/sfi.h>
#include <asm/tables.h>

u8 table_compute_checksum(void *v, int len)
{
	u8 *bytes = v;
	u8 checksum = 0;
	int i;

	for (i = 0; i < len; i++)
		checksum -= bytes[i];

	return checksum;
}

void write_tables(void)
{
	u32 __maybe_unused rom_table_end = ROM_TABLE_ADDR;

#ifdef CONFIG_GENERATE_PIRQ_TABLE
	rom_table_end = write_pirq_routing_table(rom_table_end);
	rom_table_end = ALIGN(rom_table_end, 1024);
#endif
#ifdef CONFIG_GENERATE_SFI_TABLE
	rom_table_end = write_sfi_table(rom_table_end);
	rom_table_end = ALIGN(rom_table_end, 1024);
#endif
}

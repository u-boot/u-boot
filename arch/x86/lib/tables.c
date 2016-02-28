/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/sfi.h>
#include <asm/mpspec.h>
#include <asm/smbios.h>
#include <asm/tables.h>
#include <asm/acpi_table.h>

/**
 * Function prototype to write a specific configuration table
 *
 * @addr:	start address to write the table
 * @return:	end address of the table
 */
typedef u32 (*table_write)(u32 addr);

static table_write table_write_funcs[] = {
#ifdef CONFIG_GENERATE_PIRQ_TABLE
	write_pirq_routing_table,
#endif
#ifdef CONFIG_GENERATE_SFI_TABLE
	write_sfi_table,
#endif
#ifdef CONFIG_GENERATE_MP_TABLE
	write_mp_table,
#endif
#ifdef CONFIG_GENERATE_ACPI_TABLE
	write_acpi_tables,
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	write_smbios_table,
#endif
};

u8 table_compute_checksum(void *v, int len)
{
	u8 *bytes = v;
	u8 checksum = 0;
	int i;

	for (i = 0; i < len; i++)
		checksum -= bytes[i];

	return checksum;
}

void table_fill_string(char *dest, const char *src, size_t n, char pad)
{
	int start, len;
	int i;

	strncpy(dest, src, n);

	/* Fill the remaining bytes with pad */
	len = strlen(src);
	start = len < n ? len : n;
	for (i = start; i < n; i++)
		dest[i] = pad;
}

void write_tables(void)
{
	u32 rom_table_start = ROM_TABLE_ADDR;
	u32 rom_table_end;
	int i;

	for (i = 0; i < ARRAY_SIZE(table_write_funcs); i++) {
		rom_table_end = table_write_funcs[i](rom_table_start);
		rom_table_end = ALIGN(rom_table_end, ROM_TABLE_ALIGN);
		rom_table_start = rom_table_end;
	}
}

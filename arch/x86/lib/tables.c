// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <smbios.h>
#include <acpi/acpi_table.h>
#include <asm/sfi.h>
#include <asm/mpspec.h>
#include <asm/tables.h>
#include <asm/coreboot_tables.h>

/**
 * Function prototype to write a specific configuration table
 *
 * @addr:	start address to write the table
 * @return:	end address of the table
 */
typedef ulong (*table_write)(ulong addr);

/**
 * struct table_info - Information about each table to write
 *
 * @name: Name of table (for debugging)
 * @write: Function to call to write this table
 */
struct table_info {
	const char *name;
	table_write write;
};

static struct table_info table_list[] = {
#ifdef CONFIG_GENERATE_PIRQ_TABLE
	{ "pirq", write_pirq_routing_table },
#endif
#ifdef CONFIG_GENERATE_SFI_TABLE
	{ "sfi", write_sfi_table, },
#endif
#ifdef CONFIG_GENERATE_MP_TABLE
	{ "mp", write_mp_table, },
#endif
#ifdef CONFIG_GENERATE_ACPI_TABLE
	{ "acpi", write_acpi_tables, },
#endif
#ifdef CONFIG_GENERATE_SMBIOS_TABLE
	{ "smbios", write_smbios_table, },
#endif
};

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
#ifdef CONFIG_SEABIOS
	u32 high_table, table_size;
	struct memory_area cfg_tables[ARRAY_SIZE(table_list) + 1];
#endif
	int i;

	debug("Writing tables to %x:\n", rom_table_start);
	for (i = 0; i < ARRAY_SIZE(table_list); i++) {
		const struct table_info *table = &table_list[i];

		rom_table_end = table->write(rom_table_start);
		rom_table_end = ALIGN(rom_table_end, ROM_TABLE_ALIGN);

#ifdef CONFIG_SEABIOS
		table_size = rom_table_end - rom_table_start;
		high_table = (u32)high_table_malloc(table_size);
		if (high_table) {
			table->write(high_table);

			cfg_tables[i].start = high_table;
			cfg_tables[i].size = table_size;
		} else {
			printf("%d: no memory for configuration tables\n", i);
		}
#endif

		debug("- wrote '%s' to %x, end %x\n", table->name,
		      rom_table_start, rom_table_end);
		rom_table_start = rom_table_end;
	}

#ifdef CONFIG_SEABIOS
	/* make sure the last item is zero */
	cfg_tables[i].size = 0;
	write_coreboot_table(CB_TABLE_ADDR, cfg_tables);
#endif
	debug("- done writing tables\n");
}

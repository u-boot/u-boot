// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * The 'smbios' command displays information from the SMBIOS table.
 *
 * Copyright (c) 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>
 */

#include <command.h>
#include <hexdump.h>
#include <mapmem.h>
#include <smbios.h>
#include <tables_csum.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const struct str_lookup_table wakeup_type_strings[] = {
	{ SMBIOS_WAKEUP_TYPE_RESERVED,		"Reserved" },
	{ SMBIOS_WAKEUP_TYPE_OTHER,		"Other" },
	{ SMBIOS_WAKEUP_TYPE_UNKNOWN,		"Unknown" },
	{ SMBIOS_WAKEUP_TYPE_APM_TIMER,		"APM Timer" },
	{ SMBIOS_WAKEUP_TYPE_MODEM_RING,	"Modem Ring" },
	{ SMBIOS_WAKEUP_TYPE_LAN_REMOTE,	"Lan Remote" },
	{ SMBIOS_WAKEUP_TYPE_POWER_SWITCH,	"Power Switch" },
	{ SMBIOS_WAKEUP_TYPE_PCI_PME,		"PCI PME#" },
	{ SMBIOS_WAKEUP_TYPE_AC_POWER_RESTORED,	"AC Power Restored" },
};

static const struct str_lookup_table boardtype_strings[] = {
	{ SMBIOS_BOARD_TYPE_UNKNOWN,		"Unknown" },
	{ SMBIOS_BOARD_TYPE_OTHER,		"Other" },
	{ SMBIOS_BOARD_TYPE_SERVER_BLADE,	"Server Blade" },
	{ SMBIOS_BOARD_TYPE_CON_SWITCH,		"Connectivity Switch" },
	{ SMBIOS_BOARD_TYPE_SM_MODULE,		"System Management Module" },
	{ SMBIOS_BOARD_TYPE_PROCESSOR_MODULE,	"Processor Module" },
	{ SMBIOS_BOARD_TYPE_IO_MODULE,		"I/O Module" },
	{ SMBIOS_BOARD_TYPE_MEM_MODULE,		"Memory Module" },
	{ SMBIOS_BOARD_TYPE_DAUGHTER_BOARD,	"Daughter board" },
	{ SMBIOS_BOARD_TYPE_MOTHERBOARD,	"Motherboard" },
	{ SMBIOS_BOARD_TYPE_PROC_MEM_MODULE,	"Processor/Memory Module" },
	{ SMBIOS_BOARD_TYPE_PROC_IO_MODULE,	"Processor/IO Module" },
	{ SMBIOS_BOARD_TYPE_INTERCON,		"Interconnect board" },
};

static const struct str_lookup_table chassis_state_strings[] = {
	{ SMBIOS_STATE_OTHER,		"Other" },
	{ SMBIOS_STATE_UNKNOWN,		"Unknown" },
	{ SMBIOS_STATE_SAFE,		"Safe" },
	{ SMBIOS_STATE_WARNING,		"Warning" },
	{ SMBIOS_STATE_CRITICAL,	"Critical" },
	{ SMBIOS_STATE_NONRECOVERABLE,	"Non-recoverable" },
};

static const struct str_lookup_table chassis_security_strings[] = {
	{ SMBIOS_SECURITY_OTHER,	"Other" },
	{ SMBIOS_SECURITY_UNKNOWN,	"Unknown" },
	{ SMBIOS_SECURITY_NONE,		"None" },
	{ SMBIOS_SECURITY_EXTINT_LOCK,	"External interface locked out" },
	{ SMBIOS_SECURITY_EXTINT_EN,	"External interface enabled" },
};

static const struct str_lookup_table processor_type_strings[] = {
	{ SMBIOS_PROCESSOR_TYPE_OTHER,		"Other" },
	{ SMBIOS_PROCESSOR_TYPE_UNKNOWN,	"Unknown" },
	{ SMBIOS_PROCESSOR_TYPE_CENTRAL,	"Central Processor" },
	{ SMBIOS_PROCESSOR_TYPE_MATH,		"Math Processor" },
	{ SMBIOS_PROCESSOR_TYPE_DSP,		"DSP Processor" },
	{ SMBIOS_PROCESSOR_TYPE_VIDEO,		"Video Processor" },
};

static const struct str_lookup_table processor_family_strings[] = {
	{ SMBIOS_PROCESSOR_FAMILY_OTHER,	"Other" },
	{ SMBIOS_PROCESSOR_FAMILY_UNKNOWN,	"Unknown" },
	{ SMBIOS_PROCESSOR_FAMILY_RSVD,		"Reserved" },
	{ SMBIOS_PROCESSOR_FAMILY_ARMV7,	"ARMv7" },
	{ SMBIOS_PROCESSOR_FAMILY_ARMV8,	"ARMv8" },
	{ SMBIOS_PROCESSOR_FAMILY_RV32,		"RISC-V RV32" },
	{ SMBIOS_PROCESSOR_FAMILY_RV64,		"RISC-V RV64" },
};

static const struct str_lookup_table processor_upgrade_strings[] = {
	{ SMBIOS_PROCESSOR_UPGRADE_OTHER,	"Other" },
	{ SMBIOS_PROCESSOR_UPGRADE_UNKNOWN,	"Unknown" },
	{ SMBIOS_PROCESSOR_UPGRADE_NONE,	"None" },
};

static const struct str_lookup_table err_corr_type_strings[] = {
	{ SMBIOS_CACHE_ERRCORR_OTHER,	"Other" },
	{ SMBIOS_CACHE_ERRCORR_UNKNOWN,	"Unknown" },
	{ SMBIOS_CACHE_ERRCORR_NONE,	"None" },
	{ SMBIOS_CACHE_ERRCORR_PARITY,	"Parity" },
	{ SMBIOS_CACHE_ERRCORR_SBITECC,	"Single-bit ECC" },
	{ SMBIOS_CACHE_ERRCORR_MBITECC,	"Multi-bit ECC" },
};

static const struct str_lookup_table sys_cache_type_strings[] = {
	{ SMBIOS_CACHE_SYSCACHE_TYPE_OTHER,	"Other" },
	{ SMBIOS_CACHE_SYSCACHE_TYPE_UNKNOWN,	"Unknown" },
	{ SMBIOS_CACHE_SYSCACHE_TYPE_INST,	"Instruction" },
	{ SMBIOS_CACHE_SYSCACHE_TYPE_DATA,	"Data" },
	{ SMBIOS_CACHE_SYSCACHE_TYPE_UNIFIED,	"Unified" },
};

static const struct str_lookup_table associativity_strings[] = {
	{ SMBIOS_CACHE_ASSOC_OTHER,	"Other" },
	{ SMBIOS_CACHE_ASSOC_UNKNOWN,	"Unknown" },
	{ SMBIOS_CACHE_ASSOC_DMAPPED,	"Direct Mapped" },
	{ SMBIOS_CACHE_ASSOC_2WAY,	"2-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_4WAY,	"4-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_FULLY,	"Fully Associative" },
	{ SMBIOS_CACHE_ASSOC_8WAY,	"8-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_16WAY,	"16-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_12WAY,	"12-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_24WAY,	"24-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_32WAY,	"32-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_48WAY,	"48-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_64WAY,	"64-way Set-Associative" },
	{ SMBIOS_CACHE_ASSOC_20WAY,	"20-way Set-Associative" },

};

/**
 * smbios_get_string() - get SMBIOS string from table
 *
 * @table:	SMBIOS table
 * @index:	index of the string
 * Return:	address of string, may point to empty string
 */
static const char *smbios_get_string(void *table, int index)
{
	const char *str = (char *)table +
			  ((struct smbios_header *)table)->length;
	static const char fallback[] = "";

	if (!index)
		return fallback;

	if (!*str)
		++str;
	for (--index; *str && index; --index)
		str += strlen(str) + 1;

	return str;
}

static struct smbios_header *next_table(struct smbios_header *table)
{
	const char *str;

	if (table->type == SMBIOS_END_OF_TABLE)
		return NULL;

	str = smbios_get_string(table, -1);
	return (struct smbios_header *)(++str);
}

static void smbios_print_generic(struct smbios_header *table)
{
	char *str = (char *)table + table->length;

	if (CONFIG_IS_ENABLED(HEXDUMP)) {
		printf("Header and Data:\n");
		print_hex_dump("\t", DUMP_PREFIX_OFFSET, 16, 1,
			       table, table->length, false);
	}
	if (*str) {
		printf("Strings:\n");
		for (int index = 1; *str; ++index) {
			printf("\tString %u: %s\n", index, str);
			str += strlen(str) + 1;
		}
	}
}

static void smbios_print_str(const char *label, void *table, u8 index)
{
	printf("\t%s: %s\n", label, smbios_get_string(table, index));
}

static void smbios_print_lookup_str(const struct str_lookup_table *table,
				    u16 index, u16 array_size,
				    const char *prefix)
{
	int i;
	const char *str = NULL;

	for (i = 0; i < array_size; i++) {
		if ((table + i)->idx == index)
			str = (table + i)->str;
	}

	if (str)
		printf("\t%s: %s\n", prefix, str);
	else
		printf("\t%s: [%04x]\n", prefix, index);
}

static void smbios_print_type0(struct smbios_type0 *table)
{
	printf("BIOS Information\n");
	smbios_print_str("Vendor", table, table->vendor);
	smbios_print_str("BIOS Version", table, table->bios_ver);
	/* Keep table->bios_start_segment as 0 for UEFI-based systems */
	smbios_print_str("BIOS Release Date", table, table->bios_release_date);
	printf("\tBIOS ROM Size: 0x%02x\n", table->bios_rom_size);
	printf("\tBIOS Characteristics: 0x%016llx\n",
	       table->bios_characteristics);
	printf("\tBIOS Characteristics Extension Byte 1: 0x%02x\n",
	       table->bios_characteristics_ext1);
	printf("\tBIOS Characteristics Extension Byte 2: 0x%02x\n",
	       table->bios_characteristics_ext2);
	printf("\tSystem BIOS Major Release: 0x%02x\n",
	       table->bios_major_release);
	printf("\tSystem BIOS Minor Release: 0x%02x\n",
	       table->bios_minor_release);
	printf("\tEmbedded Controller Firmware Major Release: 0x%02x\n",
	       table->ec_major_release);
	printf("\tEmbedded Controller Firmware Minor Release: 0x%02x\n",
	       table->ec_minor_release);
	printf("\tExtended BIOS ROM Size: 0x%04x\n",
	       table->extended_bios_rom_size);
}

static void smbios_print_type1(struct smbios_type1 *table)
{
	printf("System Information\n");
	smbios_print_str("Manufacturer", table, table->manufacturer);
	smbios_print_str("Product Name", table, table->product_name);
	smbios_print_str("Version", table, table->version);
	smbios_print_str("Serial Number", table, table->serial_number);
	if (table->hdr.length >= SMBIOS_TYPE1_LENGTH_V21) {
		printf("\tUUID: %pUl\n", table->uuid);
		smbios_print_lookup_str(wakeup_type_strings,
					table->wakeup_type,
					ARRAY_SIZE(wakeup_type_strings),
					"Wake-up Type");
	}
	if (table->hdr.length >= SMBIOS_TYPE1_LENGTH_V24) {
		smbios_print_str("SKU Number", table, table->sku_number);
		smbios_print_str("Family", table, table->family);
	}
}

static void smbios_print_type2(struct smbios_type2 *table)
{
	int i;
	u8 *addr = (u8 *)table + offsetof(struct smbios_type2, eos);

	printf("Baseboard Information\n");
	smbios_print_str("Manufacturer", table, table->manufacturer);
	smbios_print_str("Product Name", table, table->product_name);
	smbios_print_str("Version", table, table->version);
	smbios_print_str("Serial Number", table, table->serial_number);
	smbios_print_str("Asset Tag", table, table->asset_tag_number);
	printf("\tFeature Flags: 0x%02x\n", table->feature_flags);
	smbios_print_str("Chassis Location", table, table->chassis_location);
	printf("\tChassis Handle: 0x%04x\n", table->chassis_handle);
	smbios_print_lookup_str(boardtype_strings,
				table->board_type,
				ARRAY_SIZE(boardtype_strings),
				"Board Type");
	printf("\tNumber of Contained Object Handles: 0x%02x\n",
	       table->number_contained_objects);
	if (!table->number_contained_objects)
		return;

	printf("\tContained Object Handles:\n");
	for (i = 0; i < table->number_contained_objects; i++) {
		printf("\t\tObject[%03d]:\n", i);
		if (CONFIG_IS_ENABLED(HEXDUMP))
			print_hex_dump("\t\t", DUMP_PREFIX_OFFSET, 16, 1, addr,
				       sizeof(u16), false);
		addr += sizeof(u16);
	}
	printf("\n");
}

static void smbios_print_type3(struct smbios_type3 *table)
{
	int i;
	u8 *addr = (u8 *)table + offsetof(struct smbios_type3, sku_number);

	printf("Chassis Information\n");
	smbios_print_str("Manufacturer", table, table->manufacturer);
	printf("\tType: 0x%02x\n", table->chassis_type);
	smbios_print_str("Version", table, table->version);
	smbios_print_str("Serial Number", table, table->serial_number);
	smbios_print_str("Asset Tag", table, table->asset_tag_number);
	smbios_print_lookup_str(chassis_state_strings,
				table->bootup_state,
				ARRAY_SIZE(chassis_state_strings),
				"Boot-up State");
	smbios_print_lookup_str(chassis_state_strings,
				table->power_supply_state,
				ARRAY_SIZE(chassis_state_strings),
				"Power Supply State");
	smbios_print_lookup_str(chassis_state_strings,
				table->thermal_state,
				ARRAY_SIZE(chassis_state_strings),
				"Thermal State");
	smbios_print_lookup_str(chassis_security_strings,
				table->security_status,
				ARRAY_SIZE(chassis_security_strings),
				"Security Status");
	printf("\tOEM-defined: 0x%08x\n", table->oem_defined);
	printf("\tHeight: 0x%02x\n", table->height);
	printf("\tNumber of Power Cords: 0x%02x\n",
	       table->number_of_power_cords);
	printf("\tContained Element Count: 0x%02x\n", table->element_count);
	printf("\tContained Element Record Length: 0x%02x\n",
	       table->element_record_length);
	if (table->element_count) {
		printf("\tContained Elements:\n");
		for (i = 0; i < table->element_count; i++) {
			printf("\t\tElement[%03d]:\n", i);
			if (CONFIG_IS_ENABLED(HEXDUMP))
				print_hex_dump("\t\t", DUMP_PREFIX_OFFSET, 16,
					       1, addr,
					       table->element_record_length,
					       false);
			printf("\t\tContained Element Type: 0x%02x\n", *addr);
			printf("\t\tContained Element Minimum: 0x%02x\n",
			       *(addr + 1));
			printf("\t\tContained Element Maximum: 0x%02x\n",
			       *(addr + 2));
			addr += table->element_record_length;
		}
	}
	smbios_print_str("SKU Number", table, *addr);
}

static void smbios_print_type4(struct smbios_type4 *table)
{
	printf("Processor Information:\n");
	smbios_print_str("Socket Designation", table, table->socket_design);
	smbios_print_lookup_str(processor_type_strings,
				table->processor_type,
				ARRAY_SIZE(processor_type_strings),
				"Processor Type");
	smbios_print_lookup_str(processor_family_strings,
				table->processor_family,
				ARRAY_SIZE(processor_family_strings),
				"Processor Family");
	smbios_print_str("Processor Manufacturer", table,
			 table->processor_manufacturer);
	printf("\tProcessor ID word 0: 0x%08x\n", table->processor_id[0]);
	printf("\tProcessor ID word 1: 0x%08x\n", table->processor_id[1]);
	smbios_print_str("Processor Version", table, table->processor_version);
	printf("\tVoltage: 0x%02x\n", table->voltage);
	printf("\tExternal Clock: 0x%04x\n", table->external_clock);
	printf("\tMax Speed: 0x%04x\n", table->max_speed);
	printf("\tCurrent Speed: 0x%04x\n", table->current_speed);
	printf("\tStatus: 0x%02x\n", table->status);
	smbios_print_lookup_str(processor_upgrade_strings,
				table->processor_upgrade,
				ARRAY_SIZE(processor_upgrade_strings),
				"Processor Upgrade");
	printf("\tL1 Cache Handle: 0x%04x\n", table->l1_cache_handle);
	printf("\tL2 Cache Handle: 0x%04x\n", table->l2_cache_handle);
	printf("\tL3 Cache Handle: 0x%04x\n", table->l3_cache_handle);
	smbios_print_str("Serial Number", table, table->serial_number);
	smbios_print_str("Asset Tag", table, table->asset_tag);
	smbios_print_str("Part Number", table, table->part_number);
	printf("\tCore Count: 0x%02x\n", table->core_count);
	printf("\tCore Enabled: 0x%02x\n", table->core_enabled);
	printf("\tThread Count: 0x%02x\n", table->thread_count);
	printf("\tProcessor Characteristics: 0x%04x\n",
	       table->processor_characteristics);
	smbios_print_lookup_str(processor_family_strings,
				table->processor_family2,
				ARRAY_SIZE(processor_family_strings),
				"Processor Family 2");
	printf("\tCore Count 2: 0x%04x\n", table->core_count2);
	printf("\tCore Enabled 2: 0x%04x\n", table->core_enabled2);
	printf("\tThread Count 2: 0x%04x\n", table->thread_count2);
	printf("\tThread Enabled: 0x%04x\n", table->thread_enabled);
}

static void smbios_print_type7(struct smbios_type7 *table)
{
	printf("Cache Information:\n");
	smbios_print_str("Socket Designation", table,
			 table->socket_design);
	printf("\tCache Configuration: 0x%04x\n", table->config.data);
	printf("\tMaximum Cache Size: 0x%04x\n", table->max_size.data);
	printf("\tInstalled Size: 0x%04x\n", table->inst_size.data);
	printf("\tSupported SRAM Type: 0x%04x\n", table->supp_sram_type.data);
	printf("\tCurrent SRAM Type: 0x%04x\n", table->curr_sram_type.data);
	printf("\tCache Speed: 0x%02x\n", table->speed);
	smbios_print_lookup_str(err_corr_type_strings,
				table->err_corr_type,
				ARRAY_SIZE(err_corr_type_strings),
				"Error Correction Type");
	smbios_print_lookup_str(sys_cache_type_strings,
				table->sys_cache_type,
				ARRAY_SIZE(sys_cache_type_strings),
				"System Cache Type");
	smbios_print_lookup_str(associativity_strings,
				table->associativity,
				ARRAY_SIZE(associativity_strings),
				"Associativity");
	printf("\tMaximum Cache Size 2: 0x%08x\n", table->max_size2.data);
	printf("\tInstalled Cache Size 2: 0x%08x\n", table->inst_size2.data);
}

static void smbios_print_type127(struct smbios_type127 *table)
{
	printf("End Of Table\n");
}

static int do_smbios(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	ulong addr;
	void *entry;
	u32 size;
	char version[12];
	struct smbios_header *table;
	static const char smbios_sig[] = "_SM_";
	static const char smbios3_sig[] = "_SM3_";
	size_t count = 0;
	u32 table_maximum_size;

	addr = gd_smbios_start();
	if (!addr) {
		log_warning("SMBIOS not available\n");
		return CMD_RET_FAILURE;
	}
	entry = map_sysmem(addr, 0);
	if (!memcmp(entry, smbios3_sig, sizeof(smbios3_sig) - 1)) {
		struct smbios3_entry *entry3 = entry;

		table = (void *)(uintptr_t)entry3->struct_table_address;
		snprintf(version, sizeof(version), "%d.%d.%d",
			 entry3->major_ver, entry3->minor_ver, entry3->doc_rev);
		table = (void *)(uintptr_t)entry3->struct_table_address;
		size = entry3->length;
		table_maximum_size = entry3->table_maximum_size;
	} else if (!memcmp(entry, smbios_sig, sizeof(smbios_sig) - 1)) {
		struct smbios_entry *entry2 = entry;

		snprintf(version, sizeof(version), "%d.%d",
			 entry2->major_ver, entry2->minor_ver);
		table = (void *)(uintptr_t)entry2->struct_table_address;
		size = entry2->length;
		table_maximum_size = entry2->struct_table_length;
	} else {
		log_err("Unknown SMBIOS anchor format\n");
		return CMD_RET_FAILURE;
	}
	if (table_compute_checksum(entry, size)) {
		log_err("Invalid anchor checksum\n");
		return CMD_RET_FAILURE;
	}
	printf("SMBIOS %s present.\n", version);

	for (struct smbios_header *pos = table; pos; pos = next_table(pos))
		++count;
	printf("%zd structures occupying %d bytes\n", count, table_maximum_size);
	printf("Table at 0x%llx\n", (unsigned long long)map_to_sysmem(table));

	for (struct smbios_header *pos = table; pos; pos = next_table(pos)) {
		printf("\nHandle 0x%04x, DMI type %d, %d bytes at 0x%llx\n",
		       pos->handle, pos->type, pos->length,
		       (unsigned long long)map_to_sysmem(pos));
		switch (pos->type) {
		case SMBIOS_BIOS_INFORMATION:
			smbios_print_type0((struct smbios_type0 *)pos);
			break;
		case SMBIOS_SYSTEM_INFORMATION:
			smbios_print_type1((struct smbios_type1 *)pos);
			break;
		case SMBIOS_BOARD_INFORMATION:
			smbios_print_type2((struct smbios_type2 *)pos);
			break;
		case SMBIOS_SYSTEM_ENCLOSURE:
			smbios_print_type3((struct smbios_type3 *)pos);
			break;
		case SMBIOS_PROCESSOR_INFORMATION:
			smbios_print_type4((struct smbios_type4 *)pos);
			break;
		case SMBIOS_CACHE_INFORMATION:
			smbios_print_type7((struct smbios_type7 *)pos);
			break;
		case SMBIOS_END_OF_TABLE:
			smbios_print_type127((struct smbios_type127 *)pos);
			break;
		default:
			smbios_print_generic(pos);
			break;
		}
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_LONGHELP(smbios, "- display SMBIOS information");

U_BOOT_CMD(smbios, 1, 0, do_smbios, "display SMBIOS information",
	   smbios_help_text);

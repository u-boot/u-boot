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

static const char * const wakeup_type_strings[] = {
	"Reserved",		/* 0x00 */
	"Other",		/* 0x01 */
	"Unknown",		/* 0x02 */
	"APM Timer",		/* 0x03 */
	"Modem Ring",		/* 0x04 */
	"Lan Remote",		/* 0x05 */
	"Power Switch",		/* 0x06 */
	"PCI PME#",		/* 0x07 */
	"AC Power Restored",	/* 0x08 */
};

static const char * const boardtype_strings[] = {
	"Reserved",			/* 0x00 */
	"Unknown",			/* 0x01 */
	"Other",			/* 0x02 */
	"Server Blade",			/* 0x03 */
	"Connectivity Switch",		/* 0x04 */
	"System Management Module",	/* 0x05 */
	"Processor Module",		/* 0x06 */
	"I/O Module",			/* 0x07 */
	"Memory Module",		/* 0x08 */
	"Daughter board",		/* 0x09 */
	"Motherboard",			/* 0x0a */
	"Processor/Memory Module",	/* 0x0b */
	"Processor/IO Module",		/* 0x0c */
	"Interconnect board",		/* 0x0d */
};

static const char * const chassis_state_strings[] = {
	"Reserved",			/* 0x00 */
	"Other",			/* 0x01 */
	"Unknown",			/* 0x02 */
	"Safe",				/* 0x03 */
	"Warning",			/* 0x04 */
	"Critical",			/* 0x05 */
	"Non-recoverable",		/* 0x06 */
};

static const char * const chassis_security_strings[] = {
	"Reserved",			/* 0x00 */
	"Other",			/* 0x01 */
	"Unknown",			/* 0x02 */
	"None",				/* 0x03 */
	"External interface locked out",/* 0x04 */
	"External interface enabled",	/* 0x05 */
};

static const char * const processor_type_strings[] = {
	"Reserved",			/* 0x00 */
	"Other",			/* 0x01 */
	"Unknown",			/* 0x02 */
	"Central Processor",		/* 0x03 */
	"Math Processor",		/* 0x04 */
	"DSP Processor",		/* 0x05 */
	"Video Processor",		/* 0x06 */
};

static const char * const processor_family_strings[] = {
	[0] = "Other",
	[1] = "Unknown",
	[2 ... 253] = "Other", /* skip these definitions from now */
	[254] = "Refer to 'Processor Family 2'",
	[255] = "Reserved",
	[256] = "ARMv7",
	[257] = "ARMv8",
};

static const char * const processor_upgrade_strings[] = {
	[0] = "Reserved",
	[1] = "Other",
	[2] = "Unknown",
	[3 ... 5] = "Other", /* skip these definitions from now */
	[6] = "None",
	[7 ... 80] = "Other", /* skip these definitions from now */
};

static const char * const err_corr_type_strings[] = {
	"Reserved",		/* 0x00 */
	"Other",		/* 0x01 */
	"Unknown",		/* 0x02 */
	"None",			/* 0x03 */
	"Parity",		/* 0x04 */
	"Single-bit ECC",	/* 0x05 */
	"Multi-bit ECC",	/* 0x06 */
};

static const char * const sys_cache_type_strings[] = {
	"Reserved",		/* 0x00 */
	"Other",		/* 0x01 */
	"Unknown",		/* 0x02 */
	"Instruction",		/* 0x03 */
	"Data",			/* 0x04 */
	"Unified",		/* 0x05 */
};

static const char * const associativity_strings[] = {
	"Reserved",			/* 0x00 */
	"Other",			/* 0x01 */
	"Unknown",			/* 0x02 */
	"Direct Mapped",		/* 0x03 */
	"2-way Set-Associative",	/* 0x04 */
	"4-way Set-Associative",	/* 0x05 */
	"Fully Associative",		/* 0x06 */
	"8-way Set-Associative",	/* 0x07 */
	"16-way Set-Associative",	/* 0x08 */
	"12-way Set-Associative",	/* 0x09 */
	"24-way Set-Associative",	/* 0x0a */
	"32-way Set-Associative",	/* 0x0b */
	"48-way Set-Associative",	/* 0x0c */
	"64-way Set-Associative",	/* 0x0d */
	"20-way Set-Associative",	/* 0x0e */
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
	static const char fallback[] = "Not Specified";

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

void smbios_print_str(const char *label, void *table, u8 index)
{
	printf("\t%s: %s\n", label, smbios_get_string(table, index));
}

const char *smbios_wakeup_type_str(u8 wakeup_type)
{
	if (wakeup_type >= ARRAY_SIZE(wakeup_type_strings))
		/* Values over 0x08 are reserved. */
		wakeup_type = 0;
	return wakeup_type_strings[wakeup_type];
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
		printf("\tWake-up Type: %s\n",
		       smbios_wakeup_type_str(table->wakeup_type));
	}
	if (table->hdr.length >= SMBIOS_TYPE1_LENGTH_V24) {
		smbios_print_str("SKU Number", table, table->sku_number);
		smbios_print_str("Family", table, table->family);
	}
}

const char *smbios_baseboard_type_str(u8 borad_type)
{
	if (borad_type >= ARRAY_SIZE(boardtype_strings))
		borad_type = 0;
	return boardtype_strings[borad_type];
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
	printf("\tBoard Type: %s\n",
	       smbios_baseboard_type_str(table->board_type));
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

const char *smbios_chassis_state_str(u8 state)
{
	if (state >= ARRAY_SIZE(chassis_state_strings))
		state = 0;
	return chassis_state_strings[state];
}

const char *smbios_chassis_security_str(u8 status)
{
	if (status >= ARRAY_SIZE(chassis_security_strings))
		status = 0;
	return chassis_security_strings[status];
}

static void smbios_print_type3(struct smbios_type3 *table)
{
	int i;
	u8 *addr = (u8 *)table + offsetof(struct smbios_type3, sku_number);

	printf("Baseboard Information\n");
	smbios_print_str("Manufacturer", table, table->manufacturer);
	printf("\tType: 0x%02x\n", table->chassis_type);
	smbios_print_str("Version", table, table->version);
	smbios_print_str("Serial Number", table, table->serial_number);
	smbios_print_str("Asset Tag", table, table->asset_tag_number);

	printf("\tBoot-up State: %s\n",
	       smbios_chassis_state_str(table->bootup_state));
	printf("\tPower Supply State: %s\n",
	       smbios_chassis_state_str(table->power_supply_state));
	printf("\tThermal State: %s\n",
	       smbios_chassis_state_str(table->thermal_state));
	printf("\tSecurity Status: %s\n",
	       smbios_chassis_security_str(table->security_status));

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

const char *smbios_processor_type_str(u8 type)
{
	if (type >= ARRAY_SIZE(processor_type_strings))
		type = 0;
	return processor_type_strings[type];
}

const char *smbios_processor_family_str(u16 family)
{
	if (family >= ARRAY_SIZE(processor_family_strings))
		family = 0;

	return processor_family_strings[family];
}

const char *smbios_processor_upgrade_str(u16 upgrade)
{
	if (upgrade >= ARRAY_SIZE(processor_upgrade_strings))
		upgrade = 0;

	return processor_upgrade_strings[upgrade];
}

static void smbios_print_type4(struct smbios_type4 *table)
{
	printf("Processor Information:\n");
	smbios_print_str("Socket Designation", table, table->socket_design);
	printf("\tProcessor Type: %s\n",
	       smbios_processor_type_str(table->processor_type));
	printf("\tProcessor Family: %s\n",
	       smbios_processor_family_str(table->processor_family));
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
	printf("\tProcessor Upgrade: %s\n",
	       smbios_processor_upgrade_str(table->processor_upgrade));
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
	printf("\tProcessor Family 2: %s\n",
	       smbios_processor_family_str(table->processor_family2));
	printf("\tCore Count 2: 0x%04x\n", table->core_count2);
	printf("\tCore Enabled 2: 0x%04x\n", table->core_enabled2);
	printf("\tThread Count 2: 0x%04x\n", table->thread_count2);
	printf("\tThread Enabled: 0x%04x\n", table->thread_enabled);
}

const char *smbios_cache_err_corr_type_str(u8 err_corr_type)
{
	if (err_corr_type >= ARRAY_SIZE(err_corr_type_strings))
		err_corr_type = 0;
	return err_corr_type_strings[err_corr_type];
}

const char *smbios_cache_sys_cache_type_str(u8 sys_cache_type)
{
	if (sys_cache_type >= ARRAY_SIZE(sys_cache_type_strings))
		sys_cache_type = 0;
	return sys_cache_type_strings[sys_cache_type];
}

const char *smbios_cache_associativity_str(u8 associativity)
{
	if (associativity >= ARRAY_SIZE(associativity_strings))
		associativity = 0;
	return associativity_strings[associativity];
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
	printf("\tError Correction Type: %s\n",
	       smbios_cache_err_corr_type_str(table->err_corr_type));
	printf("\tSystem Cache Type: %s\n",
	       smbios_cache_sys_cache_type_str(table->sys_cache_type));
	printf("\tAssociativity: %s\n",
	       smbios_cache_associativity_str(table->associativity));
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

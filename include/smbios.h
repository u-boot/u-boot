/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * Adapted from coreboot src/include/smbios.h
 */

#ifndef _SMBIOS_H_
#define _SMBIOS_H_

#include <linux/types.h>
#include <smbios_def.h>

/* SMBIOS spec version implemented */
#define SMBIOS_MAJOR_VER	3
#define SMBIOS_MINOR_VER	7

enum {
	SMBIOS_STR_MAX	= 64,	/* Maximum length allowed for a string */
};

/* SMBIOS structure types */
enum {
	SMBIOS_BIOS_INFORMATION = 0,
	SMBIOS_SYSTEM_INFORMATION = 1,
	SMBIOS_BOARD_INFORMATION = 2,
	SMBIOS_SYSTEM_ENCLOSURE = 3,
	SMBIOS_PROCESSOR_INFORMATION = 4,
	SMBIOS_CACHE_INFORMATION = 7,
	SMBIOS_SYSTEM_SLOTS = 9,
	SMBIOS_PHYS_MEMORY_ARRAY = 16,
	SMBIOS_MEMORY_DEVICE = 17,
	SMBIOS_MEMORY_ARRAY_MAPPED_ADDRESS = 19,
	SMBIOS_SYSTEM_BOOT_INFORMATION = 32,
	SMBIOS_END_OF_TABLE = 127
};

#define SMBIOS_INTERMEDIATE_OFFSET	16
#define SMBIOS_STRUCT_EOS_BYTES		2

struct str_lookup_table {
	u16 idx;
	const char *str;
};

struct __packed smbios_entry {
	u8 anchor[4];
	u8 checksum;
	u8 length;
	u8 major_ver;
	u8 minor_ver;
	u16 max_struct_size;
	u8 entry_point_rev;
	u8 formatted_area[5];
	u8 intermediate_anchor[5];
	u8 intermediate_checksum;
	u16 struct_table_length;
	u32 struct_table_address;
	u16 struct_count;
	u8 bcd_rev;
};

/**
 * struct smbios3_entry - SMBIOS 3.0 (64-bit) Entry Point structure
 */
struct __packed smbios3_entry {
	/** @anchor: anchor string */
	u8 anchor[5];
	/** @checksum: checksum of the entry point structure */
	u8 checksum;
	/** @length: length of the entry point structure */
	u8 length;
	/** @major_ver: major version of the SMBIOS specification */
	u8 major_ver;
	/** @minor_ver: minor version of the SMBIOS specification */
	u8 minor_ver;
	/** @docrev: revision of the SMBIOS specification */
	u8 doc_rev;
	/** @entry_point_rev: revision of the entry point structure */
	u8 entry_point_rev;
	/** @reserved: reserved */
	u8 reserved;
	/** maximum size of SMBIOS table */
	u32 table_maximum_size;
	/** @struct_table_address: 64-bit physical starting address */
	u64 struct_table_address;
};

struct __packed smbios_header {
	u8 type;
	u8 length;
	u16 handle;
};

struct __packed smbios_type0 {
	struct smbios_header hdr;
	u8 vendor;
	u8 bios_ver;
	u16 bios_start_segment;
	u8 bios_release_date;
	u8 bios_rom_size;
	u64 bios_characteristics;
	u8 bios_characteristics_ext1;
	u8 bios_characteristics_ext2;
	u8 bios_major_release;
	u8 bios_minor_release;
	u8 ec_major_release;
	u8 ec_minor_release;
	u16 extended_bios_rom_size;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

#define SMBIOS_TYPE1_LENGTH_V20		0x08
#define SMBIOS_TYPE1_LENGTH_V21		0x19
#define SMBIOS_TYPE1_LENGTH_V24		0x1b

struct __packed smbios_type1 {
	struct smbios_header hdr;
	u8 manufacturer;
	u8 product_name;
	u8 version;
	u8 serial_number;
	u8 uuid[16];
	u8 wakeup_type;
	u8 sku_number;
	u8 family;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

#define SMBIOS_TYPE2_CON_OBJ_HANDLE_SIZE sizeof(u16)

struct __packed smbios_type2 {
	struct smbios_header hdr;
	u8 manufacturer;
	u8 product_name;
	u8 version;
	u8 serial_number;
	u8 asset_tag_number;
	u8 feature_flags;
	u8 chassis_location;
	u16 chassis_handle;
	u8 board_type;
	u8 number_contained_objects;
	/*
	 * Dynamic bytes will be inserted here to store the objects.
	 * length is equal to 'number_contained_objects'.
	 */
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

struct __packed smbios_type3 {
	struct smbios_header hdr;
	u8 manufacturer;
	u8 chassis_type;
	u8 version;
	u8 serial_number;
	u8 asset_tag_number;
	u8 bootup_state;
	u8 power_supply_state;
	u8 thermal_state;
	u8 security_status;
	u32 oem_defined;
	u8 height;
	u8 number_of_power_cords;
	u8 element_count;
	u8 element_record_length;
	/*
	 * Dynamic bytes will be inserted here to store the elements.
	 * length is equal to 'element_record_length' * 'element_record_length'
	 */
	u8 sku_number;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

struct __packed smbios_type4 {
	struct smbios_header hdr;
	u8 socket_design;
	u8 processor_type;
	u8 processor_family;
	u8 processor_manufacturer;
	u32 processor_id[2];
	u8 processor_version;
	u8 voltage;
	u16 external_clock;
	u16 max_speed;
	u16 current_speed;
	u8 status;
	u8 processor_upgrade;
	u16 l1_cache_handle;
	u16 l2_cache_handle;
	u16 l3_cache_handle;
	u8 serial_number;
	u8 asset_tag;
	u8 part_number;
	u8 core_count;
	u8 core_enabled;
	u8 thread_count;
	u16 processor_characteristics;
	u16 processor_family2;
	u16 core_count2;
	u16 core_enabled2;
	u16 thread_count2;
	u16 thread_enabled;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

union cache_config {
	struct {
		u16 level:3;
		u16 bsocketed:1;
		u16 rsvd0:1;
		u16 locate:2;
		u16 benabled:1;
		u16 opmode:2;
		u16 rsvd1:6;
	} fields;
	u16 data;
};

union cache_size_word {
	struct {
		u16 size:15;
		u16 granu:1;
	} fields;
	u16 data;
};

union cache_size_dword {
	struct {
		u32 size:31;
		u32 granu:1;
	} fields;
	u32 data;
};

union cache_sram_type {
	struct {
		u16 other:1;
		u16 unknown:1;
		u16 nonburst:1;
		u16 burst:1;
		u16 plburst:1;
		u16 sync:1;
		u16 async:1;
		u16 rsvd:9;
	} fields;
	u16 data;
};

struct __packed smbios_type7 {
	struct smbios_header hdr;
	u8 socket_design;
	union cache_config config;
	union cache_size_word max_size;
	union cache_size_word inst_size;
	union cache_sram_type supp_sram_type;
	union cache_sram_type curr_sram_type;
	u8 speed;
	u8 err_corr_type;
	u8 sys_cache_type;
	u8 associativity;
	union cache_size_dword max_size2;
	union cache_size_dword inst_size2;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

struct __packed smbios_type32 {
	u8 type;
	u8 length;
	u16 handle;
	u8 reserved[6];
	u8 boot_status;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

struct __packed smbios_type127 {
	u8 type;
	u8 length;
	u16 handle;
	char eos[SMBIOS_STRUCT_EOS_BYTES];
};

/**
 * fill_smbios_header() - Fill the header of an SMBIOS table
 *
 * This fills the header of an SMBIOS table structure.
 *
 * @table:	start address of the structure
 * @type:	the type of structure
 * @length:	the length of the formatted area of the structure
 * @handle:	the structure's handle, a unique 16-bit number
 */
static inline void fill_smbios_header(void *table, int type,
				      int length, int handle)
{
	struct smbios_header *header = table;

	header->type = type;
	header->length = length - SMBIOS_STRUCT_EOS_BYTES;
	header->handle = handle;
}

/**
 * write_smbios_table() - Write SMBIOS table
 *
 * This writes SMBIOS table at a given address.
 *
 * @addr:	start address to write SMBIOS table, 16-byte-alignment
 * recommended. Note that while the SMBIOS tables themself have no alignment
 * requirement, some systems may requires alignment. For example x86 systems
 * which put tables at f0000 require 16-byte alignment
 *
 * Return:	end address of SMBIOS table (and start address for next entry)
 *		or NULL in case of an error
 */
ulong write_smbios_table(ulong addr);

/**
 * smbios_entry() - Get a valid struct smbios_entry pointer
 *
 * @address:   address where smbios tables is located
 * @size:      size of smbios table
 * @return:    NULL or a valid pointer to a struct smbios_entry
 */
const struct smbios_entry *smbios_entry(u64 address, u32 size);

/**
 * smbios_header() - Search for SMBIOS header type
 *
 * @entry:     pointer to a struct smbios_entry
 * @type:      SMBIOS type
 * @return:    NULL or a valid pointer to a struct smbios_header
 */
const struct smbios_header *smbios_header(const struct smbios_entry *entry, int type);

/**
 * smbios_string() - Return string from SMBIOS
 *
 * @header:    pointer to struct smbios_header
 * @index:     string index
 * @return:    NULL or a valid char pointer
 */
char *smbios_string(const struct smbios_header *header, int index);

/**
 * smbios_update_version() - Update the version string
 *
 * This can be called after the SMBIOS tables are written (e.g. after the U-Boot
 * main loop has started) to update the BIOS version string (SMBIOS table 0).
 *
 * @version: New version string to use
 * Return: 0 if OK, -ENOENT if no version string was previously written,
 *	-ENOSPC if the new string is too large to fit
 */
int smbios_update_version(const char *version);

/**
 * smbios_update_version_full() - Update the version string
 *
 * This can be called after the SMBIOS tables are written (e.g. after the U-Boot
 * main loop has started) to update the BIOS version string (SMBIOS table 0).
 * It scans for the correct place to put the version, so does not need U-Boot
 * to have actually written the tables itself (e.g. if a previous bootloader
 * did it).
 *
 * @smbios_tab: Start of SMBIOS tables
 * @version: New version string to use
 * Return: 0 if OK, -ENOENT if no version string was previously written,
 *	-ENOSPC if the new string is too large to fit
 */
int smbios_update_version_full(void *smbios_tab, const char *version);

/**
 * smbios_prepare_measurement() - Update smbios table for the measurement
 *
 * TCG specification requires to measure static configuration information.
 * This function clear the device dependent parameters such as
 * serial number for the measurement.
 *
 * @entry: pointer to a struct smbios3_entry
 * @header: pointer to a struct smbios_header
 */
void smbios_prepare_measurement(const struct smbios3_entry *entry,
				struct smbios_header *header);

#endif /* _SMBIOS_H_ */

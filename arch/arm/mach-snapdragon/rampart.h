// SPDX-License-Identifier: GPL-2.0
/*
 * RAM partition table definitions
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 */

#ifndef __QCOM_SMEM_RAMPART_H__
#define __QCOM_SMEM_RAMPART_H__

#include <linux/types.h>

#define SMEM_USABLE_RAM_PARTITION_TABLE		402

#define RAM_PARTITION_H_MAJOR  03
#define RAM_PARTITION_H_MINOR  00

/**
 * Total length of zero filled name string. This is not a C
 * string, as it can occupy the total number of bytes, and if
 * it does, it does not require a zero terminator. It cannot
 * be manipulated with standard string handling library functions.
 */
#define RAM_PART_NAME_LENGTH 16

/**
 * Number of RAM partition entries which are usable by APPS.
 */
#define RAM_NUM_PART_ENTRIES 32

/**
 * @name: Magic numbers
 * Used in identifying valid RAM partition table.
 */
#define RAM_PART_MAGIC1     0x9da5e0a8
#define RAM_PART_MAGIC2     0xaf9ec4e2

/**
 * RAM partition attributes.
 */
enum ram_partition_attribute_t {
	RAM_PARTITION_READ_ONLY = 0,       /* Read-only RAM partition */
	RAM_PARTITION_READWRITE,           /* Read/write RAM partition */
};

/**
 * RAM partition categories.
 */
enum ram_partition_category_t {
	RAM_PARTITION_IRAM = 4,                   /* IRAM RAM partition */
	RAM_PARTITION_IMEM = 5,                   /* IMEM RAM partition */
	RAM_PARTITION_SDRAM = 14,                  /* SDRAM type without specific bus information**/
};

/**
 * RAM Partition domains.
 * @note: For shared RAM partition, domain value would be 0b11:\n
 * RAM_PARTITION_APPS_DOMAIN | RAM_PARTITION_MODEM_DOMAIN.
 */
enum ram_partition_domain_t {
	RAM_PARTITION_DEFAULT_DOMAIN = 0,  /* 0b00: No specific domain definition */
	RAM_PARTITION_APPS_DOMAIN = 1,     /* 0b01: APPS RAM partition */
	RAM_PARTITION_MODEM_DOMAIN = 2,    /* 0b10: MODEM RAM partition */
};

/**
 * RAM Partition types.
 * @note: The RAM_PARTITION_SYS_MEMORY type represents DDR rams that are attached
 * to the current system.
 */
enum ram_partition_type_t {
	RAM_PARTITION_SYS_MEMORY = 1,        /* system memory */
	RAM_PARTITION_BOOT_REGION_MEMORY1,   /* boot loader memory 1 */
	RAM_PARTITION_BOOT_REGION_MEMORY2,   /* boot loader memory 2, reserved */
	RAM_PARTITION_APPSBL_MEMORY,         /* apps boot loader memory */
	RAM_PARTITION_APPS_MEMORY,           /* apps usage memory */
	RAM_PARTITION_TOOLS_FV_MEMORY,       /* tools usage memory */
	RAM_PARTITION_QUANTUM_FV_MEMORY,     /* quantum usage memory */
	RAM_PARTITION_QUEST_FV_MEMORY,       /* quest usage memory */
};

/* Common table header between versions */
struct usable_ram_partition_table_header {
	u32 magic1;          /* Magic number to identify valid RAM partition table */
	u32 magic2;          /* Magic number to identify valid RAM partition table */
	u32 version;         /* Version number to track structure definition changes */
	u32 reserved1;       /* Reserved for future use */

	u32 num_partitions;  /* Number of RAM partition table entries */
};

/* Holds information for an entry in the RAM partition table */
struct ram_partition_entry_v3 {
	char name[RAM_PART_NAME_LENGTH];  /* Partition name, unused for now */
	u64 start_address;                /* Partition start address in RAM */
	u64 length;                       /* Partition length in RAM in Bytes */
	u32 partition_attribute;          /* Partition attribute */
	u32 partition_category;           /* Partition category */
	u32 partition_domain;             /* Partition domain */
	u32 partition_type;               /* Partition type */
	u32 num_partitions;               /* Number of partitions on device */
	u32 hw_info;                      /* hw information such as type and frequency */
	u8 highest_bank_bit;              /* Highest bit corresponding to a bank */
	u8 reserve0;                      /* Reserved for future use */
	u8 reserve1;                      /* Reserved for future use */
	u8 reserve2;                      /* Reserved for future use */
	u32 min_pasr_size;                /* Minimum PASR size in MB */
	u64 available_length;             /* Available Partition length in RAM in Bytes */
};

/*
 * Defines the RAM partition table structure
 *
 * Do not change the placement of the first four elements so that future
 * compatibility will always be guaranteed at least for the identifiers.
 *
 * The other portion of the structure may be changed as necessary to accommodate
 * new features. Be sure to increment version number if you change it.
 */
struct usable_ram_partition_table_v3 {
	struct usable_ram_partition_table_header header;

	u32 reserved2;       /* Added for 8 bytes alignment of header */

	/* RAM partition table entries */
	struct ram_partition_entry_v3 entries[RAM_NUM_PART_ENTRIES];
};

/* Version 1 structure 32 Bit - Holds information for an entry in the RAM partition table */
struct ram_partition_entry_v1 {
	char name[RAM_PART_NAME_LENGTH];  /* Partition name, unused for now */
	u64 start_address;                /* Partition start address in RAM */
	u64 length;                       /* Partition length in RAM in Bytes */
	u32 partition_attribute;          /* Partition attribute */
	u32 partition_category;           /* Partition category */
	u32 partition_domain;             /* Partition domain */
	u32 partition_type;               /* Partition type */
	u32 num_partitions;               /* Number of partitions on device */
	u32 hw_info;                      /* hw information such as type and frequency */
	u32 reserved4;                    /* Reserved for future use */
	u32 reserved5;                    /* Reserved for future use */
};

/*
 * Defines the RAM partition table structure (Version 1)
 *
 * Do not change the placement of the first four elements so that future
 * compatibility will always be guaranteed at least for the identifiers.
 *
 * The other portion of the structure may be changed as necessary to accommodate
 * new features. Be sure to increment version number if you change it.
 */
struct usable_ram_partition_table_v1 {
	struct usable_ram_partition_table_header header;

	u32 reserved2;       /* Added for 8 bytes alignment of header */

	/* RAM partition table entries */
	struct ram_partition_entry_v1 entries[RAM_NUM_PART_ENTRIES];
};

/* Version 0 structure 32 Bit - Holds information for an entry in the RAM partition table */
struct ram_partition_entry_v0 {
	char name[RAM_PART_NAME_LENGTH];  /* Partition name, unused for now */
	u32 start_address;                /* Partition start address in RAM */
	u32 length;                       /* Partition length in RAM in Bytes */
	u32 partition_attribute;          /* Partition attribute */
	u32 partition_category;           /* Partition category */
	u32 partition_domain;             /* Partition domain */
	u32 partition_type;               /* Partition type */
	u32 num_partitions;               /* Number of partitions on device */
	u32 reserved3;                    /* Reserved for future use */
	u32 reserved4;                    /* Reserved for future use */
	u32 reserved5;                    /* Reserved for future use */
};

/*
 * Defines the RAM partition table structure (Version 0)
 *
 * Do not change the placement of the first four elements so that future
 * compatibility will always be guaranteed at least for the identifiers.
 *
 * The other portion of the structure may be changed as necessary to accommodate
 * new features. Be sure to increment version number if you change it.
 */
struct usable_ram_partition_table_v0 {
	struct usable_ram_partition_table_header header;

	/* RAM partition table entries */
	struct ram_partition_entry_v0 entries[RAM_NUM_PART_ENTRIES];
};

#endif // __QCOM_SMEM_RAMPART_H__

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * NBX Freebox Serial Info Support
 *
 * Copyright (C) 2025 Free Mobile, Freebox
 *
 * Reads device serial number and MAC address from eMMC.
 * Used to identify the board and set network MAC addresses.
 */

#ifndef NBX_FBXSERIAL_H
#define NBX_FBXSERIAL_H

#include <linux/types.h>

/*
 * Extended info structure - variable data depending on type
 */
#define NBX_EXTINFO_SIZE		128
#define NBX_EXTINFO_MAX_COUNT		16

/* Extended info types */
#define NBX_EXTINFO_TYPE_EXTDEV		1

/* Extended device types */
#define NBX_EXTDEV_TYPE_BUNDLE		1
#define NBX_EXTDEV_TYPE_MAX		2

struct nbx_serial_extinfo {
	u32 type;

	union {
		/* extdev */
		struct {
			u32 type;
			u32 model;
			char serial[64];
		} extdev;

		/* raw access */
		unsigned char data[NBX_EXTINFO_SIZE];
	} u;
} __packed;

/*
 * Master serial structure
 */
#define NBX_FBXSERIAL_VERSION		1
#define NBX_FBXSERIAL_MAGIC		0x2d9521ab

#define NBX_MAC_ADDR_SIZE		6
#define NBX_RANDOM_DATA_SIZE		32

/* Maximum size for CRC validation */
#define NBX_FBXSERIAL_MAX_SIZE		8192

struct nbx_fbx_serial {
	u32 crc32;
	u32 magic;
	u32 struct_version;
	u32 len;

	/* Board serial */
	u16 type;
	u8 version;
	u8 manufacturer;
	u16 year;
	u8 week;
	u32 number;
	u32 flags;

	/* MAC address base */
	u8 mac_addr_base[NBX_MAC_ADDR_SIZE];

	/* MAC address count */
	u8 mac_count;

	/* Random data used to derive keys */
	u8 random_data[NBX_RANDOM_DATA_SIZE];

	/* Last update of data (seconds since epoch) */
	u32 last_modified;

	/* Count of following extinfo tags */
	u32 extinfo_count;

	/* Beginning of extended info */
	struct nbx_serial_extinfo extinfos[NBX_EXTINFO_MAX_COUNT];
} __packed;

/**
 * nbx_fbxserial_set_default() - Initialize serial structure with defaults
 * @serial: Pointer to serial structure to initialize
 *
 * Sets the serial structure to default values (Freebox OUI, type 9018).
 * Used as fallback when serial info cannot be read from eMMC.
 */
static inline void nbx_fbxserial_set_default(struct nbx_fbx_serial *serial)
{
	static const struct nbx_fbx_serial def = {
		.crc32		= 0,
		.magic		= NBX_FBXSERIAL_MAGIC,
		.struct_version	= NBX_FBXSERIAL_VERSION,
		.len		= sizeof(struct nbx_fbx_serial),
		.type		= 9018,
		.version	= 0,
		.manufacturer	= '_',
		.year		= 0,
		.week		= 0,
		.number		= 0,
		.flags		= 0,
		.mac_addr_base	= { 0x00, 0x07, 0xCB, 0x00, 0x00, 0xFD },
		.mac_count	= 1,
		.random_data	= { 0 },
		.last_modified	= 0,
		.extinfo_count	= 0,
	};

	memcpy(serial, &def, sizeof(def));
}

/**
 * nbx_fbx_read_serial() - Read serial info from eMMC
 * @dev_num: MMC device number
 * @offset: Byte offset in eMMC where serial info is stored
 * @fs: Pointer to serial structure to fill
 *
 * Reads and validates the serial info from eMMC. On failure,
 * the structure is filled with default values.
 *
 * Return: 0 on success, negative on error (defaults still set)
 */
int nbx_fbx_read_serial(int dev_num, unsigned long offset,
			struct nbx_fbx_serial *fs);

/**
 * nbx_fbx_dump_serial() - Print serial info to console
 * @fs: Pointer to serial structure to display
 *
 * Prints the serial number, MAC address, and bundle info (if present).
 */
void nbx_fbx_dump_serial(struct nbx_fbx_serial *fs);

/**
 * nbx_fbx_init_ethaddr() - Initialize Ethernet addresses from serial info
 * @dev_num: MMC device number
 * @offset: Byte offset in eMMC where serial info is stored
 *
 * Reads serial info and sets ethaddr, eth1addr, eth2addr environment
 * variables from the MAC address in the serial structure.
 *
 * Return: 0 on success, negative on error
 */
int nbx_fbx_init_ethaddr(int dev_num, unsigned long offset);

#endif /* NBX_FBXSERIAL_H */

/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Daniel Schultz <d.schultz@phytec.de>
 */

#ifndef _PHYTEC_SOM_DETECTION_BLOCKS_H
#define _PHYTEC_SOM_DETECTION_BLOCKS_H

#define PHYTEC_API3_DATA_HEADER_LEN	8
#define PHYTEC_API3_BLOCK_HEADER_LEN	4
#define PHYTEC_API3_PAYLOAD_START					      \
	(PHYTEC_API2_DATA_LEN + PHYTEC_API3_DATA_HEADER_LEN)

#define PHYTEC_API3_ELEMENT_HEADER_SIZE					      \
	(sizeof(struct phytec_api3_element *) +				      \
		sizeof(enum phytec_api3_block_types))

#define PHYTEC_API3_FOREACH_BLOCK(elem, data)				      \
	for (elem = phytec_get_block_head(data); elem; elem = elem->next)

struct phytec_api3_header {
	u16 data_length;	/* Total length in Bytes of all blocks */
	u8 block_count;		/* Number of blocks */
	u8 sub_version;		/* Block specification version */
	u8 reserved[3];		/* Reserved */
	u8 crc8;		/* checksum */
} __packed;

struct phytec_api3_block_header {
	u8 block_type;		/* Block payload identifier */
	u16 next_block;		/* Address of the next block */
	u8 crc8;		/* checksum */
} __packed;

enum phytec_api3_block_types {
	PHYTEC_API3_BLOCK_MAC = 0,
};

struct phytec_api3_block_mac {
	u8 interface;		/* Ethernet interface number */
	u8 address[6];		/* MAC-Address */
	u8 crc8;		/* checksum */
} __packed;

struct phytec_api3_element {
	struct phytec_api3_element *next;
	enum phytec_api3_block_types block_type;
	union {
		struct phytec_api3_block_mac mac;
	} block;
} __packed;

struct phytec_api3_element *
	phytec_blocks_init_mac(struct phytec_api3_block_header *header,
			       uint8_t *payload);

int __maybe_unused
phytec_blocks_add_mac_to_env(struct phytec_api3_element *element);

#endif /* _PHYTEC_SOM_DETECTION_BLOCKS_H */

// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2024 PHYTEC Messtechnik GmbH
 * Author: Daniel Schultz <d.schultz@phytec.de>
 */

#include <env.h>
#include <malloc.h>
#include <u-boot/crc.h>
#include <net.h>
#include <vsprintf.h>

#include "phytec_som_detection_blocks.h"

#if IS_ENABLED(CONFIG_PHYTEC_SOM_DETECTION_BLOCKS)

struct phytec_api3_element *
	phytec_blocks_init_mac(struct phytec_api3_block_header *header,
			       uint8_t *payload)
{
	struct phytec_api3_element *element;
	struct phytec_api3_block_mac *mac;
	unsigned int crc;
	unsigned int len = sizeof(struct phytec_api3_block_mac);

	if (!header)
		return NULL;
	if (!payload)
		return NULL;

	element = (struct phytec_api3_element *)
			calloc(8, PHYTEC_API3_ELEMENT_HEADER_SIZE + len);
	if (!element) {
		pr_err("%s: Unable to allocate memory\n", __func__);
		return NULL;
	}
	element->block_type = header->block_type;
	memcpy(&element->block.mac, payload, len);
	mac = &element->block.mac;

	debug("%s: interface: %i\n", __func__, mac->interface);
	debug("%s: MAC %pM\n", __func__, mac->address);

	crc = crc8(0, (const unsigned char *)mac, len);
	debug("%s: crc: %x\n", __func__, crc);
	if (crc) {
		pr_err("%s: CRC mismatch. API3 block payload is unusable\n",
		       __func__);
		return NULL;
	}

	return element;
}

int __maybe_unused
	phytec_blocks_add_mac_to_env(struct phytec_api3_element *element)
{
	char enetenv[9] = "ethaddr";
	char buf[ARP_HLEN_ASCII + 1];
	struct phytec_api3_block_mac *block = &element->block.mac;
	int ret;

	if (!is_valid_ethaddr(block->address)) {
		pr_err("%s: Invalid MAC address in block.\n", __func__);
		return -1;
	}

	if (block->interface > 0) {
		ret = sprintf(enetenv, "eth%iaddr", block->interface);
		if (ret != 8) {
			pr_err("%s: Unable to create env string\n", __func__);
			return -1;
		}
	}

	ret = sprintf(buf, "%pM", block->address);
	if (ret != ARP_HLEN_ASCII) {
		pr_err("%s: Unable to convert MAC address\n", __func__);
		return -1;
	}
	ret = env_set(enetenv, buf);
	if (ret) {
		pr_err("%s: Failed to set MAC address to env.\n", __func__);
		return -1;
	}

	debug("%s: Added %s to %s\n", __func__, buf, enetenv);
	return 0;
}

#else

inline struct phytec_api3_element *
	phytec_api3_init_mac_block(struct phytec_api3_block_header *header,
				   uint8_t *payload)
{
	return NULL;
}

inline int __maybe_unused
	phytec_blocks_add_mac_to_env(struct phytec_api3_element *element)
{
	return -1;
}

#endif

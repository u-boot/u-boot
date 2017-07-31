/*
 *  EFI device path interface
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>

#define MAC_OUTPUT_LEN 22
#define UNKNOWN_OUTPUT_LEN 23

const efi_guid_t efi_guid_device_path_to_text_protocol =
		EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;

static uint16_t *efi_convert_device_node_to_text(
		struct efi_device_path_protocol *device_node,
		bool display_only,
		bool allow_shortcuts)
{
	unsigned long buffer_size;
	efi_status_t r;
	uint16_t *buffer = NULL;
	int i;

	switch (device_node->type) {
	case DEVICE_PATH_TYPE_END:
		return NULL;
	case DEVICE_PATH_TYPE_MESSAGING_DEVICE:
		switch (device_node->sub_type) {
		case DEVICE_PATH_SUB_TYPE_MSG_MAC_ADDR: {
			struct efi_device_path_mac_addr *dp =
				(struct efi_device_path_mac_addr *)device_node;

			if (dp->if_type != 0 && dp->if_type != 1)
				break;
			r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES,
					      2 * MAC_OUTPUT_LEN,
					      (void **)&buffer);
			if (r != EFI_SUCCESS)
				return NULL;
			sprintf((char *)buffer,
				"MAC(%02x%02x%02x%02x%02x%02x,0x%1x)",
				dp->mac.addr[0], dp->mac.addr[1],
				dp->mac.addr[2], dp->mac.addr[3],
				dp->mac.addr[4], dp->mac.addr[5],
				dp->if_type);
			for (i = MAC_OUTPUT_LEN - 1; i >= 0; --i)
				buffer[i] = ((uint8_t *)buffer)[i];
			break;
			}
		}
		break;
	case DEVICE_PATH_TYPE_MEDIA_DEVICE:
		switch (device_node->sub_type) {
		case DEVICE_PATH_SUB_TYPE_FILE_PATH:
			buffer_size = device_node->length - 4;
			r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES,
					      buffer_size, (void **) &buffer);
			if (r != EFI_SUCCESS)
				return NULL;
			memcpy(buffer, device_node->data, buffer_size);
			break;
		}
		break;
	}

	/*
	 * For all node types that we do not yet support return
	 * 'UNKNOWN(type,subtype)'.
	 */
	if (!buffer) {
		r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES,
				      2 * UNKNOWN_OUTPUT_LEN,
				      (void **)&buffer);
		if (r != EFI_SUCCESS)
			return NULL;
		sprintf((char *)buffer,
			"UNKNOWN(%04x,%04x)",
			device_node->type,
			device_node->sub_type);
		for (i = UNKNOWN_OUTPUT_LEN - 1; i >= 0; --i)
			buffer[i] = ((uint8_t *)buffer)[i];
	}

	return buffer;
}

static uint16_t EFIAPI *efi_convert_device_node_to_text_ext(
		struct efi_device_path_protocol *device_node,
		bool display_only,
		bool allow_shortcuts)
{
	uint16_t *buffer;

	EFI_ENTRY("%p, %d, %d", device_node, display_only, allow_shortcuts);

	buffer = efi_convert_device_node_to_text(device_node, display_only,
						 allow_shortcuts);

	EFI_EXIT(EFI_SUCCESS);
	return buffer;
}

static uint16_t EFIAPI *efi_convert_device_path_to_text(
		struct efi_device_path_protocol *device_path,
		bool display_only,
		bool allow_shortcuts)
{
	uint16_t *buffer;

	EFI_ENTRY("%p, %d, %d", device_path, display_only, allow_shortcuts);

	/*
	 * Our device paths are all of depth one. So its is sufficient to
	 * to convert the first node.
	 */
	buffer = efi_convert_device_node_to_text(device_path, display_only,
						 allow_shortcuts);

	EFI_EXIT(EFI_SUCCESS);
	return buffer;
}

const struct efi_device_path_to_text_protocol efi_device_path_to_text = {
	.convert_device_node_to_text = efi_convert_device_node_to_text_ext,
	.convert_device_path_to_text = efi_convert_device_path_to_text,
};

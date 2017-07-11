/*
 *  EFI device path interface
 *
 *  Copyright (c) 2017 Heinrich Schuchardt
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <efi_loader.h>

#define MEDIA_DEVICE_PATH 4
#define FILE_PATH_MEDIA_DEVICE_PATH 4

const efi_guid_t efi_guid_device_path_to_text_protocol =
		EFI_DEVICE_PATH_TO_TEXT_PROTOCOL_GUID;

uint16_t *efi_convert_device_node_to_text(
		struct efi_device_path_protocol *device_node,
		bool display_only,
		bool allow_shortcuts)
{
	EFI_ENTRY("%p, %d, %d", device_node, display_only, allow_shortcuts);

	EFI_EXIT(EFI_UNSUPPORTED);
	return NULL;
}

uint16_t *efi_convert_device_path_to_text(
		struct efi_device_path_protocol *device_path,
		bool display_only,
		bool allow_shortcuts)
{
	EFI_ENTRY("%p, %d, %d", device_path, display_only, allow_shortcuts);

	unsigned long buffer_size;
	efi_status_t r;
	uint16_t *buffer = NULL;

	switch (device_path->type) {
	case MEDIA_DEVICE_PATH:
		switch (device_path->sub_type) {
		case FILE_PATH_MEDIA_DEVICE_PATH:
			buffer_size = device_path->length - 4;
			r = efi_allocate_pool(EFI_ALLOCATE_ANY_PAGES,
					      buffer_size, (void **) &buffer);
			if (r == EFI_SUCCESS)
				memcpy(buffer, device_path->data, buffer_size);
			break;
		}
	}

	if (buffer) {
		EFI_EXIT(EFI_SUCCESS);
	} else {
		debug("type %d, subtype %d\n",
		      device_path->type, device_path->sub_type);
		EFI_EXIT(EFI_UNSUPPORTED);
	}

	return buffer;
}

const struct efi_device_path_to_text_protocol efi_device_path_to_text = {
	.convert_device_node_to_text = efi_convert_device_node_to_text,
	.convert_device_path_to_text = efi_convert_device_path_to_text,
};

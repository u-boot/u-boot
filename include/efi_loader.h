/*
 *  EFI application loader
 *
 *  Copyright (c) 2016 Alexander Graf
 *
 *  SPDX-License-Identifier:     GPL-2.0+
 */

#include <part_efi.h>
#include <efi_api.h>
#include <linux/list.h>

extern const efi_guid_t efi_guid_device_path;
extern const efi_guid_t efi_guid_loaded_image;

efi_status_t efi_return_handle(void *handle,
		efi_guid_t *protocol, void **protocol_interface,
		void *agent_handle, void *controller_handle,
		uint32_t attributes);
void *efi_load_pe(void *efi, struct efi_loaded_image *loaded_image_info);

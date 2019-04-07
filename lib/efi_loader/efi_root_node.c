// SPDX-License-Identifier: GPL-2.0+
/*
 *  Root node for system services
 *
 *  Copyright (c) 2018 Heinrich Schuchardt
 */

#include <common.h>
#include <malloc.h>
#include <efi_loader.h>

const efi_guid_t efi_u_boot_guid = U_BOOT_GUID;

struct efi_root_dp {
	struct efi_device_path_vendor vendor;
	struct efi_device_path end;
} __packed;

/**
 * efi_root_node_register() - create root node
 *
 * Create the root node on which we install all protocols that are
 * not related to a loaded image or a driver.
 *
 * Return:	status code
 */
efi_status_t efi_root_node_register(void)
{
	efi_handle_t root;
	efi_status_t ret;
	struct efi_root_dp *dp;

	/* Create handle */
	ret = efi_create_handle(&root);
	if (ret != EFI_SUCCESS)
		return ret;

	/* Install device path protocol */
	dp = calloc(1, sizeof(*dp));
	if (!dp)
		return EFI_OUT_OF_RESOURCES;

	/* Fill vendor node */
	dp->vendor.dp.type = DEVICE_PATH_TYPE_HARDWARE_DEVICE;
	dp->vendor.dp.sub_type = DEVICE_PATH_SUB_TYPE_VENDOR;
	dp->vendor.dp.length = sizeof(struct efi_device_path_vendor);
	dp->vendor.guid = efi_u_boot_guid;

	/* Fill end node */
	dp->end.type = DEVICE_PATH_TYPE_END;
	dp->end.sub_type = DEVICE_PATH_SUB_TYPE_END;
	dp->end.length = sizeof(struct efi_device_path);

	/* Install device path protocol */
	ret = efi_add_protocol(root, &efi_guid_device_path, dp);
	if (ret != EFI_SUCCESS)
		goto failure;

	/* Install device path to text protocol */
	ret = efi_add_protocol(root, &efi_guid_device_path_to_text_protocol,
			       (void *)&efi_device_path_to_text);
	if (ret != EFI_SUCCESS)
		goto failure;

	/* Install device path utilities protocol */
	ret = efi_add_protocol(root, &efi_guid_device_path_utilities_protocol,
			       (void *)&efi_device_path_utilities);
	if (ret != EFI_SUCCESS)
		goto failure;

	/* Install Unicode collation protocol */
	ret = efi_add_protocol(root, &efi_guid_unicode_collation_protocol,
			       (void *)&efi_unicode_collation_protocol);
	if (ret != EFI_SUCCESS)
		goto failure;

#if CONFIG_IS_ENABLED(EFI_LOADER_HII)
	/* Install HII string protocol */
	ret = efi_add_protocol(root, &efi_guid_hii_string_protocol,
			       (void *)&efi_hii_string);
	if (ret != EFI_SUCCESS)
		goto failure;

	/* Install HII database protocol */
	ret = efi_add_protocol(root, &efi_guid_hii_database_protocol,
			       (void *)&efi_hii_database);
	if (ret != EFI_SUCCESS)
		goto failure;

	/* Install HII configuration routing protocol */
	ret = efi_add_protocol(root, &efi_guid_hii_config_routing_protocol,
			       (void *)&efi_hii_config_routing);
	if (ret != EFI_SUCCESS)
		goto failure;
#endif

failure:
	return ret;
}

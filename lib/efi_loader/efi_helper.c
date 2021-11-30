// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Linaro Limited
 */

#define LOG_CATEGORY LOGC_EFI
#include <common.h>
#include <env.h>
#include <malloc.h>
#include <dm.h>
#include <fs.h>
#include <efi_load_initrd.h>
#include <efi_loader.h>
#include <efi_variable.h>

#if defined(CONFIG_CMD_EFIDEBUG) || defined(CONFIG_EFI_LOAD_FILE2_INITRD)
/* GUID used by Linux to identify the LoadFile2 protocol with the initrd */
const efi_guid_t efi_lf2_initrd_guid = EFI_INITRD_MEDIA_GUID;
#endif

/**
 * efi_create_current_boot_var() - Return Boot#### name were #### is replaced by
 *			           the value of BootCurrent
 *
 * @var_name:		variable name
 * @var_name_size:	size of var_name
 *
 * Return:	Status code
 */
static efi_status_t efi_create_current_boot_var(u16 var_name[],
						size_t var_name_size)
{
	efi_uintn_t boot_current_size;
	efi_status_t ret;
	u16 boot_current;
	u16 *pos;

	boot_current_size = sizeof(boot_current);
	ret = efi_get_variable_int(L"BootCurrent",
				   &efi_global_variable_guid, NULL,
				   &boot_current_size, &boot_current, NULL);
	if (ret != EFI_SUCCESS)
		goto out;

	pos = efi_create_indexed_name(var_name, var_name_size, "Boot",
				      boot_current);
	if (!pos) {
		ret = EFI_OUT_OF_RESOURCES;
		goto out;
	}

out:
	return ret;
}

/**
 * efi_get_dp_from_boot() - Retrieve and return a device path from an EFI
 *			    Boot### variable.
 *			    A boot option may contain an array of device paths.
 *			    We use a VenMedia() with a specific GUID to identify
 *			    the usage of the array members. This function is
 *			    used to extract a specific device path
 *
 * @guid:	vendor GUID of the VenMedia() device path node identifying the
 *		device path
 *
 * Return:	device path or NULL. Caller must free the returned value
 */
struct efi_device_path *efi_get_dp_from_boot(const efi_guid_t guid)
{
	struct efi_load_option lo;
	void *var_value;
	efi_uintn_t size;
	efi_status_t ret;
	u16 var_name[16];

	ret = efi_create_current_boot_var(var_name, sizeof(var_name));
	if (ret != EFI_SUCCESS)
		return NULL;

	var_value = efi_get_var(var_name, &efi_global_variable_guid, &size);
	if (!var_value)
		return NULL;

	ret = efi_deserialize_load_option(&lo, var_value, &size);
	if (ret != EFI_SUCCESS)
		goto err;

	return efi_dp_from_lo(&lo, &guid);

err:
	free(var_value);
	return NULL;
}

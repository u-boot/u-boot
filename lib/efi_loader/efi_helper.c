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
	ret = efi_get_variable_int(u"BootCurrent",
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

const struct guid_to_hash_map {
	efi_guid_t guid;
	const char algo[32];
	u32 bits;
} guid_to_hash[] = {
	{
		EFI_CERT_X509_SHA256_GUID,
		"sha256",
		SHA256_SUM_LEN * 8,
	},
	{
		EFI_CERT_SHA256_GUID,
		"sha256",
		SHA256_SUM_LEN * 8,
	},
	{
		EFI_CERT_X509_SHA384_GUID,
		"sha384",
		SHA384_SUM_LEN * 8,
	},
	{
		EFI_CERT_X509_SHA512_GUID,
		"sha512",
		SHA512_SUM_LEN * 8,
	},
};

#define MAX_GUID_TO_HASH_COUNT ARRAY_SIZE(guid_to_hash)

/** guid_to_sha_str - return the sha string e.g "sha256" for a given guid
 *                    used on EFI security databases
 *
 * @guid: guid to check
 *
 * Return: len or 0 if no match is found
 */
const char *guid_to_sha_str(const efi_guid_t *guid)
{
	size_t i;

	for (i = 0; i < MAX_GUID_TO_HASH_COUNT; i++) {
		if (!guidcmp(guid, &guid_to_hash[i].guid))
			return guid_to_hash[i].algo;
	}

	return NULL;
}

/** algo_to_len - return the sha size in bytes for a given string
 *
 * @algo: string indicating hashing algorithm to check
 *
 * Return: length of hash in bytes or 0 if no match is found
 */
int algo_to_len(const char *algo)
{
	size_t i;

	for (i = 0; i < MAX_GUID_TO_HASH_COUNT; i++) {
		if (!strcmp(algo, guid_to_hash[i].algo))
			return guid_to_hash[i].bits / 8;
	}

	return 0;
}

/** efi_link_dev - link the efi_handle_t and udevice
 *
 * @handle:	efi handle to associate with udevice
 * @dev:	udevice to associate with efi handle
 *
 * Return:	0 on success, negative on failure
 */
int efi_link_dev(efi_handle_t handle, struct udevice *dev)
{
	handle->dev = dev;
	return dev_tag_set_ptr(dev, DM_TAG_EFI, handle);
}

/**
 * efi_unlink_dev() - unlink udevice and handle
 *
 * @handle:	EFI handle to unlink
 *
 * Return:	0 on success, negative on failure
 */
int efi_unlink_dev(efi_handle_t handle)
{
	int ret;

	ret = dev_tag_del(handle->dev, DM_TAG_EFI);
	if (ret)
		return ret;
	handle->dev = NULL;

	return 0;
}

static int u16_tohex(u16 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	/* not hexadecimal */
	return -1;
}

bool efi_varname_is_load_option(u16 *var_name16, int *index)
{
	int id, i, digit;

	if (memcmp(var_name16, u"Boot", 8))
		return false;

	for (id = 0, i = 0; i < 4; i++) {
		digit = u16_tohex(var_name16[4 + i]);
		if (digit < 0)
			break;
		id = (id << 4) + digit;
	}
	if (i == 4 && !var_name16[8]) {
		if (index)
			*index = id;
		return true;
	}

	return false;
}

/**
 * efi_next_variable_name() - get next variable name
 *
 * This function is a wrapper of efi_get_next_variable_name_int().
 * If efi_get_next_variable_name_int() returns EFI_BUFFER_TOO_SMALL,
 * @size and @buf are updated by new buffer size and realloced buffer.
 *
 * @size:	pointer to the buffer size
 * @buf:	pointer to the buffer
 * @guid:	pointer to the guid
 * Return:	status code
 */
efi_status_t efi_next_variable_name(efi_uintn_t *size, u16 **buf, efi_guid_t *guid)
{
	u16 *p;
	efi_status_t ret;
	efi_uintn_t buf_size = *size;

	ret = efi_get_next_variable_name_int(&buf_size, *buf, guid);
	if (ret == EFI_NOT_FOUND)
		return ret;
	if (ret == EFI_BUFFER_TOO_SMALL) {
		p = realloc(*buf, buf_size);
		if (!p)
			return EFI_OUT_OF_RESOURCES;

		*buf = p;
		*size = buf_size;
		ret = efi_get_next_variable_name_int(&buf_size, *buf, guid);
	}

	return ret;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI boot manager
 *
 *  Copyright (c) 2017 Rob Clark
 */

#define LOG_CATEGORY LOGC_EFI

#include <common.h>
#include <charset.h>
#include <log.h>
#include <malloc.h>
#include <efi_default_filename.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <asm/unaligned.h>

static const struct efi_boot_services *bs;
static const struct efi_runtime_services *rs;

const efi_guid_t efi_guid_bootmenu_auto_generated =
		EFICONFIG_AUTO_GENERATED_ENTRY_GUID;

/*
 * bootmgr implements the logic of trying to find a payload to boot
 * based on the BootOrder + BootXXXX variables, and then loading it.
 *
 * TODO detecting a special key held (f9?) and displaying a boot menu
 * like you would get on a PC would be clever.
 *
 * TODO if we had a way to write and persist variables after the OS
 * has started, we'd also want to check OsIndications to see if we
 * should do normal or recovery boot.
 */

/**
 * expand_media_path() - expand a device path for default file name
 * @device_path:	device path to check against
 *
 * If @device_path is a media or disk partition which houses a file
 * system, this function returns a full device path which contains
 * an architecture-specific default file name for removable media.
 *
 * Return:	a newly allocated device path
 */
static
struct efi_device_path *expand_media_path(struct efi_device_path *device_path)
{
	struct efi_device_path *dp, *rem, *full_path;
	efi_handle_t handle;

	if (!device_path)
		return NULL;

	/*
	 * If device_path is a (removable) media or partition which provides
	 * simple file system protocol, append a default file name to support
	 * booting from removable media.
	 */
	dp = device_path;
	handle = efi_dp_find_obj(dp, &efi_simple_file_system_protocol_guid,
				 &rem);
	if (handle) {
		if (rem->type == DEVICE_PATH_TYPE_END) {
			dp = efi_dp_from_file(NULL, 0,
					      "/EFI/BOOT/" BOOTEFI_NAME);
			full_path = efi_dp_append(device_path, dp);
			efi_free_pool(dp);
		} else {
			full_path = efi_dp_dup(device_path);
		}
	} else {
		full_path = efi_dp_dup(device_path);
	}

	return full_path;
}

/**
 * try_load_from_file_path() - try to load a file
 *
 * Given a file media path iterate through a list of handles and try to
 * to load the file from each of them until the first success.
 *
 * @fs_handles: array of handles with the simple file protocol
 * @num:	number of handles in fs_handles
 * @fp:		file path to open
 * @handle:	on return pointer to handle for loaded image
 * @removable:	if true only consider removable media, else only non-removable
 */
static efi_status_t try_load_from_file_path(efi_handle_t *fs_handles,
					    efi_uintn_t num,
					    struct efi_device_path *fp,
					    efi_handle_t *handle,
					    bool removable)
{
	struct efi_handler *handler;
	struct efi_device_path *dp;
	int i;
	efi_status_t ret;

	for (i = 0; i < num; i++) {
		if (removable != efi_disk_is_removable(fs_handles[i]))
			continue;

		ret = efi_search_protocol(fs_handles[i], &efi_guid_device_path,
					  &handler);
		if (ret != EFI_SUCCESS)
			continue;

		dp = handler->protocol_interface;
		if (!dp)
			continue;

		dp = efi_dp_append(dp, fp);
		if (!dp)
			continue;

		ret = EFI_CALL(efi_load_image(true, efi_root, dp, NULL, 0,
					      handle));
		efi_free_pool(dp);
		if (ret == EFI_SUCCESS)
			return ret;
	}

	return EFI_NOT_FOUND;
}

/**
 * try_load_from_short_path
 * @fp:		file path
 * @handle:	pointer to handle for newly installed image
 *
 * Enumerate all the devices which support file system operations,
 * prepend its media device path to the file path, @fp, and
 * try to load the file.
 * This function should be called when handling a short-form path
 * which is starting with a file device path.
 *
 * Return:	status code
 */
static efi_status_t try_load_from_short_path(struct efi_device_path *fp,
					     efi_handle_t *handle)
{
	efi_handle_t *fs_handles;
	efi_uintn_t num;
	efi_status_t ret;

	ret = EFI_CALL(efi_locate_handle_buffer(
					BY_PROTOCOL,
					&efi_simple_file_system_protocol_guid,
					NULL,
					&num, &fs_handles));
	if (ret != EFI_SUCCESS)
		return ret;
	if (!num)
		return EFI_NOT_FOUND;

	/* removable media first */
	ret = try_load_from_file_path(fs_handles, num, fp, handle, true);
	if (ret == EFI_SUCCESS)
		goto out;

	/* fixed media */
	ret = try_load_from_file_path(fs_handles, num, fp, handle, false);
	if (ret == EFI_SUCCESS)
		goto out;

out:
	return ret;
}

/**
 * try_load_entry() - try to load image for boot option
 *
 * Attempt to load load-option number 'n', returning device_path and file_path
 * if successful. This checks that the EFI_LOAD_OPTION is active (enabled)
 * and that the specified file to boot exists.
 *
 * @n:			number of the boot option, e.g. 0x0a13 for Boot0A13
 * @handle:		on return handle for the newly installed image
 * @load_options:	load options set on the loaded image protocol
 * Return:		status code
 */
static efi_status_t try_load_entry(u16 n, efi_handle_t *handle,
				   void **load_options)
{
	struct efi_load_option lo;
	u16 varname[9];
	void *load_option;
	efi_uintn_t size;
	efi_status_t ret;

	efi_create_indexed_name(varname, sizeof(varname), "Boot", n);

	load_option = efi_get_var(varname, &efi_global_variable_guid, &size);
	if (!load_option)
		return EFI_LOAD_ERROR;

	ret = efi_deserialize_load_option(&lo, load_option, &size);
	if (ret != EFI_SUCCESS) {
		log_warning("Invalid load option for %ls\n", varname);
		goto error;
	}

	if (lo.attributes & LOAD_OPTION_ACTIVE) {
		struct efi_device_path *file_path;
		u32 attributes;

		log_debug("trying to load \"%ls\" from %pD\n", lo.label,
			  lo.file_path);

		if (EFI_DP_TYPE(lo.file_path, MEDIA_DEVICE, FILE_PATH)) {
			/* file_path doesn't contain a device path */
			ret = try_load_from_short_path(lo.file_path, handle);
		} else {
			file_path = expand_media_path(lo.file_path);
			ret = EFI_CALL(efi_load_image(true, efi_root, file_path,
						      NULL, 0, handle));
			efi_free_pool(file_path);
		}
		if (ret != EFI_SUCCESS) {
			log_warning("Loading %ls '%ls' failed\n",
				    varname, lo.label);
			goto error;
		}

		attributes = EFI_VARIABLE_BOOTSERVICE_ACCESS |
			     EFI_VARIABLE_RUNTIME_ACCESS;
		ret = efi_set_variable_int(u"BootCurrent",
					   &efi_global_variable_guid,
					   attributes, sizeof(n), &n, false);
		if (ret != EFI_SUCCESS)
			goto unload;
		/* try to register load file2 for initrd's */
		if (IS_ENABLED(CONFIG_EFI_LOAD_FILE2_INITRD)) {
			ret = efi_initrd_register();
			if (ret != EFI_SUCCESS)
				goto unload;
		}

		log_info("Booting: %ls\n", lo.label);
	} else {
		ret = EFI_LOAD_ERROR;
	}

	/* Set load options */
	if (size >= sizeof(efi_guid_t) &&
	    !guidcmp(lo.optional_data, &efi_guid_bootmenu_auto_generated))
		size = 0;

	if (size) {
		*load_options = malloc(size);
		if (!*load_options) {
			ret = EFI_OUT_OF_RESOURCES;
			goto error;
		}
		memcpy(*load_options, lo.optional_data, size);
		ret = efi_set_load_options(*handle, size, *load_options);
	} else {
		*load_options = NULL;
	}

error:
	free(load_option);

	return ret;

unload:
	if (EFI_CALL(efi_unload_image(*handle)) != EFI_SUCCESS)
		log_err("Unloading image failed\n");
	free(load_option);

	return ret;
}

/**
 * efi_bootmgr_load() - try to load from BootNext or BootOrder
 *
 * Attempt to load from BootNext or in the order specified by BootOrder
 * EFI variable, the available load-options, finding and returning
 * the first one that can be loaded successfully.
 *
 * @handle:		on return handle for the newly installed image
 * @load_options:	load options set on the loaded image protocol
 * Return:		status code
 */
efi_status_t efi_bootmgr_load(efi_handle_t *handle, void **load_options)
{
	u16 bootnext, *bootorder;
	efi_uintn_t size;
	int i, num;
	efi_status_t ret;

	bs = systab.boottime;
	rs = systab.runtime;

	/* BootNext */
	size = sizeof(bootnext);
	ret = efi_get_variable_int(u"BootNext",
				   &efi_global_variable_guid,
				   NULL, &size, &bootnext, NULL);
	if (ret == EFI_SUCCESS || ret == EFI_BUFFER_TOO_SMALL) {
		/* BootNext does exist here */
		if (ret == EFI_BUFFER_TOO_SMALL || size != sizeof(u16))
			log_err("BootNext must be 16-bit integer\n");

		/* delete BootNext */
		ret = efi_set_variable_int(u"BootNext",
					   &efi_global_variable_guid,
					   0, 0, NULL, false);

		/* load BootNext */
		if (ret == EFI_SUCCESS) {
			if (size == sizeof(u16)) {
				ret = try_load_entry(bootnext, handle,
						     load_options);
				if (ret == EFI_SUCCESS)
					return ret;
				log_warning(
					"Loading from BootNext failed, falling back to BootOrder\n");
			}
		} else {
			log_err("Deleting BootNext failed\n");
		}
	}

	/* BootOrder */
	bootorder = efi_get_var(u"BootOrder", &efi_global_variable_guid, &size);
	if (!bootorder) {
		log_info("BootOrder not defined\n");
		ret = EFI_NOT_FOUND;
		goto error;
	}

	num = size / sizeof(uint16_t);
	for (i = 0; i < num; i++) {
		log_debug("trying to load Boot%04X\n", bootorder[i]);
		ret = try_load_entry(bootorder[i], handle, load_options);
		if (ret == EFI_SUCCESS)
			break;
	}

	free(bootorder);

error:
	return ret;
}

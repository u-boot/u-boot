// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2020, Linaro Limited
 */

#include <common.h>
#include <env.h>
#include <malloc.h>
#include <mapmem.h>
#include <dm.h>
#include <fs.h>
#include <efi_loader.h>
#include <efi_load_initrd.h>

static const efi_guid_t efi_guid_load_file2_protocol =
		EFI_LOAD_FILE2_PROTOCOL_GUID;

static efi_status_t EFIAPI
efi_load_file2_initrd(struct efi_load_file_protocol *this,
		      struct efi_device_path *file_path, bool boot_policy,
		      efi_uintn_t *buffer_size, void *buffer);

static const struct efi_load_file_protocol efi_lf2_protocol = {
	.load_file = efi_load_file2_initrd,
};

/*
 * Device path defined by Linux to identify the handle providing the
 * EFI_LOAD_FILE2_PROTOCOL used for loading the initial ramdisk.
 */
static const struct efi_initrd_dp dp = {
	.vendor = {
		{
		   DEVICE_PATH_TYPE_MEDIA_DEVICE,
		   DEVICE_PATH_SUB_TYPE_VENDOR_PATH,
		   sizeof(dp.vendor),
		},
		EFI_INITRD_MEDIA_GUID,
	},
	.end = {
		DEVICE_PATH_TYPE_END,
		DEVICE_PATH_SUB_TYPE_END,
		sizeof(dp.end),
	}
};

/**
 * get_file_size() - retrieve the size of initramfs, set efi status on error
 *
 * @dev:			device to read from. i.e "mmc"
 * @part:			device partition. i.e "0:1"
 * @file:			name fo file
 * @status:			EFI exit code in case of failure
 *
 * Return:			size of file
 */
static loff_t get_file_size(const char *dev, const char *part, const char *file,
			    efi_status_t *status)
{
	loff_t sz = 0;
	int ret;

	ret = fs_set_blk_dev(dev, part, FS_TYPE_ANY);
	if (ret) {
		*status = EFI_NO_MEDIA;
		goto out;
	}

	ret = fs_size(file, &sz);
	if (ret) {
		sz = 0;
		*status = EFI_NOT_FOUND;
		goto out;
	}

out:
	return sz;
}

/**
 * load_file2() - get information about random number generation
 *
 * This function implement the LoadFile2() service in order to load an initram
 * disk requested by the Linux kernel stub.
 * See the UEFI spec for details.
 *
 * @this:			loadfile2 protocol instance
 * @file_path:			relative path of the file. "" in this case
 * @boot_policy:		must be false for Loadfile2
 * @buffer_size:		size of allocated buffer
 * @buffer:			buffer to load the file
 *
 * Return:			status code
 */
static efi_status_t EFIAPI
efi_load_file2_initrd(struct efi_load_file_protocol *this,
		      struct efi_device_path *file_path, bool boot_policy,
		      efi_uintn_t *buffer_size, void *buffer)
{
	const char *filespec = CONFIG_EFI_INITRD_FILESPEC;
	efi_status_t status = EFI_NOT_FOUND;
	loff_t file_sz = 0, read_sz = 0;
	char *dev, *part, *file;
	char *s;
	int ret;

	EFI_ENTRY("%p, %p, %d, %p, %p", this, file_path, boot_policy,
		  buffer_size, buffer);

	s = strdup(filespec);
	if (!s)
		goto out;

	if (!this || this != &efi_lf2_protocol ||
	    !buffer_size) {
		status = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (file_path->type != dp.end.type ||
	    file_path->sub_type != dp.end.sub_type) {
		status = EFI_INVALID_PARAMETER;
		goto out;
	}

	if (boot_policy) {
		status = EFI_UNSUPPORTED;
		goto out;
	}

	/* expect something like 'mmc 0:1 initrd.cpio.gz' */
	dev = strsep(&s, " ");
	if (!dev)
		goto out;
	part = strsep(&s, " ");
	if (!part)
		goto out;
	file = strsep(&s, " ");
	if (!file)
		goto out;

	file_sz = get_file_size(dev, part, file, &status);
	if (!file_sz)
		goto out;

	if (!buffer || *buffer_size < file_sz) {
		status = EFI_BUFFER_TOO_SMALL;
		*buffer_size = file_sz;
	} else {
		ret = fs_set_blk_dev(dev, part, FS_TYPE_ANY);
		if (ret) {
			status = EFI_NO_MEDIA;
			goto out;
		}

		ret = fs_read(file, map_to_sysmem(buffer), 0, *buffer_size,
			      &read_sz);
		if (ret || read_sz != file_sz)
			goto out;
		*buffer_size = read_sz;

		status = EFI_SUCCESS;
	}

out:
	free(s);
	return EFI_EXIT(status);
}

/**
 * efi_initrd_register() - Register a handle and loadfile2 protocol
 *
 * This function creates a new handle and installs a linux specific GUID
 * to handle initram disk loading during boot.
 * See the UEFI spec for details.
 *
 * Return:			status code
 */
efi_status_t efi_initrd_register(void)
{
	efi_handle_t efi_initrd_handle = NULL;
	efi_status_t ret;

	/*
	 * Set up the handle with the EFI_LOAD_FILE2_PROTOCOL which Linux may
	 * use to load the initial ramdisk.
	 */
	ret = EFI_CALL(efi_install_multiple_protocol_interfaces
		       (&efi_initrd_handle,
			/* initramfs */
			&efi_guid_device_path, &dp,
			/* LOAD_FILE2 */
			&efi_guid_load_file2_protocol,
			(void *)&efi_lf2_protocol,
			NULL));

	return ret;
}

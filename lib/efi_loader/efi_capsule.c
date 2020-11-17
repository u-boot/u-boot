// SPDX-License-Identifier: GPL-2.0+
/*
 *  EFI Capsule
 *
 *  Copyright (c) 2018 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#include <common.h>
#include <efi_loader.h>
#include <efi_variable.h>
#include <fs.h>
#include <malloc.h>
#include <mapmem.h>
#include <sort.h>

const efi_guid_t efi_guid_capsule_report = EFI_CAPSULE_REPORT_GUID;

#ifdef CONFIG_EFI_CAPSULE_ON_DISK
/* for file system access */
static struct efi_file_handle *bootdev_root;
#endif

/**
 * get_last_capsule - get the last capsule index
 *
 * Retrieve the index of the capsule invoked last time from "CapsuleLast"
 * variable.
 *
 * Return:
 * * > 0	- the last capsule index invoked
 * * 0xffff	- on error, or no capsule invoked yet
 */
static __maybe_unused unsigned int get_last_capsule(void)
{
	u16 value16[11]; /* "CapsuleXXXX": non-null-terminated */
	char value[11], *p;
	efi_uintn_t size;
	unsigned long index = 0xffff;
	efi_status_t ret;

	size = sizeof(value16);
	ret = efi_get_variable_int(L"CapsuleLast", &efi_guid_capsule_report,
				   NULL, &size, value16, NULL);
	if (ret != EFI_SUCCESS || u16_strncmp(value16, L"Capsule", 7))
		goto err;

	p = value;
	utf16_utf8_strcpy(&p, value16);
	strict_strtoul(&value[7], 16, &index);
err:
	return index;
}

/**
 * set_capsule_result - set a result variable
 * @capsule:		Capsule
 * @return_status:	Return status
 *
 * Create and set a result variable, "CapsuleXXXX", for the capsule,
 * @capsule.
 */
static __maybe_unused
void set_capsule_result(int index, struct efi_capsule_header *capsule,
			efi_status_t return_status)
{
	u16 variable_name16[12];
	struct efi_capsule_result_variable_header result;
	struct efi_time time;
	efi_status_t ret;

	efi_create_indexed_name(variable_name16, "Capsule", index);

	result.variable_total_size = sizeof(result);
	result.capsule_guid = capsule->capsule_guid;
	ret = EFI_CALL((*efi_runtime_services.get_time)(&time, NULL));
	if (ret == EFI_SUCCESS)
		memcpy(&result.capsule_processed, &time, sizeof(time));
	else
		memset(&result.capsule_processed, 0, sizeof(time));
	result.capsule_status = return_status;
	ret = efi_set_variable(variable_name16, &efi_guid_capsule_report,
			       EFI_VARIABLE_NON_VOLATILE |
			       EFI_VARIABLE_BOOTSERVICE_ACCESS |
			       EFI_VARIABLE_RUNTIME_ACCESS,
			       sizeof(result), &result);
	if (ret)
		printf("EFI: creating %ls failed\n", variable_name16);
}

/**
 * efi_update_capsule() - process information from operating system
 * @capsule_header_array:	Array of virtual address pointers
 * @capsule_count:		Number of pointers in capsule_header_array
 * @scatter_gather_list:	Array of physical address pointers
 *
 * This function implements the UpdateCapsule() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:			status code
 */
efi_status_t EFIAPI efi_update_capsule(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 scatter_gather_list)
{
	struct efi_capsule_header *capsule;
	unsigned int i;
	efi_status_t ret;

	EFI_ENTRY("%p, %lu, %llu\n", capsule_header_array, capsule_count,
		  scatter_gather_list);

	if (!capsule_count) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = EFI_UNSUPPORTED;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
	}
out:
	return EFI_EXIT(ret);
}

/**
 * efi_query_capsule_caps() - check if capsule is supported
 * @capsule_header_array:	Array of virtual pointers
 * @capsule_count:		Number of pointers in capsule_header_array
 * @maximum_capsule_size:	Maximum capsule size
 * @reset_type:			Type of reset needed for capsule update
 *
 * This function implements the QueryCapsuleCapabilities() runtime service.
 *
 * See the Unified Extensible Firmware Interface (UEFI) specification for
 * details.
 *
 * Return:			status code
 */
efi_status_t EFIAPI efi_query_capsule_caps(
		struct efi_capsule_header **capsule_header_array,
		efi_uintn_t capsule_count,
		u64 *maximum_capsule_size,
		u32 *reset_type)
{
	struct efi_capsule_header *capsule __attribute__((unused));
	unsigned int i;
	efi_status_t ret;

	EFI_ENTRY("%p, %lu, %p, %p\n", capsule_header_array, capsule_count,
		  maximum_capsule_size, reset_type);

	if (!maximum_capsule_size) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	*maximum_capsule_size = U64_MAX;
	*reset_type = EFI_RESET_COLD;

	ret = EFI_SUCCESS;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
		/* TODO */
	}
out:
	return EFI_EXIT(ret);
}

#ifdef CONFIG_EFI_CAPSULE_ON_DISK
/**
 * get_dp_device - retrieve a device  path from boot variable
 * @boot_var:	Boot variable name
 * @device_dp	Device path
 *
 * Retrieve a device patch from boot variable, @boot_var.
 *
 * Return:	status code
 */
static efi_status_t get_dp_device(u16 *boot_var,
				  struct efi_device_path **device_dp)
{
	void *buf = NULL;
	efi_uintn_t size;
	struct efi_load_option lo;
	struct efi_device_path *file_dp;
	efi_status_t ret;

	size = 0;
	ret = efi_get_variable_int(boot_var, &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		buf = malloc(size);
		if (!buf)
			return EFI_OUT_OF_RESOURCES;
		ret = efi_get_variable_int(boot_var, &efi_global_variable_guid,
					   NULL, &size, buf, NULL);
	}
	if (ret != EFI_SUCCESS)
		return ret;

	efi_deserialize_load_option(&lo, buf, &size);

	if (lo.attributes & LOAD_OPTION_ACTIVE) {
		efi_dp_split_file_path(lo.file_path, device_dp, &file_dp);
		efi_free_pool(file_dp);

		ret = EFI_SUCCESS;
	} else {
		ret = EFI_NOT_FOUND;
	}

	free(buf);

	return ret;
}

/**
 * device_is_present_and_system_part - check if a device exists
 * @dp		Device path
 *
 * Check if a device pointed to by the device path, @dp, exists and is
 * located in UEFI system partition.
 *
 * Return:	true - yes, false - no
 */
static bool device_is_present_and_system_part(struct efi_device_path *dp)
{
	efi_handle_t handle;

	handle = efi_dp_find_obj(dp, NULL);
	if (!handle)
		return false;

	return efi_disk_is_system_part(handle);
}

/**
 * find_boot_device - identify the boot device
 *
 * Identify the boot device from boot-related variables as UEFI
 * specification describes and put its handle into bootdev_root.
 *
 * Return:	status code
 */
static efi_status_t find_boot_device(void)
{
	char boot_var[9];
	u16 boot_var16[9], *p, bootnext, *boot_order = NULL;
	efi_uintn_t size;
	int i, num;
	struct efi_simple_file_system_protocol *volume;
	struct efi_device_path *boot_dev = NULL;
	efi_status_t ret;

	/* find active boot device in BootNext */
	bootnext = 0;
	size = sizeof(bootnext);
	ret = efi_get_variable_int(L"BootNext",
				   (efi_guid_t *)&efi_global_variable_guid,
				   NULL, &size, &bootnext, NULL);
	if (ret == EFI_SUCCESS || ret == EFI_BUFFER_TOO_SMALL) {
		/* BootNext does exist here */
		if (ret == EFI_BUFFER_TOO_SMALL || size != sizeof(u16)) {
			printf("BootNext must be 16-bit integer\n");
			goto skip;
		}
		sprintf((char *)boot_var, "Boot%04X", bootnext);
		p = boot_var16;
		utf8_utf16_strcpy(&p, boot_var);

		ret = get_dp_device(boot_var16, &boot_dev);
		if (ret == EFI_SUCCESS) {
			if (device_is_present_and_system_part(boot_dev)) {
				goto out;
			} else {
				efi_free_pool(boot_dev);
				boot_dev = NULL;
			}
		}
	}

skip:
	/* find active boot device in BootOrder */
	size = 0;
	ret = efi_get_variable_int(L"BootOrder", &efi_global_variable_guid,
				   NULL, &size, NULL, NULL);
	if (ret == EFI_BUFFER_TOO_SMALL) {
		boot_order = malloc(size);
		if (!boot_order) {
			ret = EFI_OUT_OF_RESOURCES;
			goto out;
		}

		ret = efi_get_variable_int(L"BootOrder",
					   &efi_global_variable_guid,
					   NULL, &size, boot_order, NULL);
	}
	if (ret != EFI_SUCCESS)
		goto out;

	/* check in higher order */
	num = size / sizeof(u16);
	for (i = 0; i < num; i++) {
		sprintf((char *)boot_var, "Boot%04X", boot_order[i]);
		p = boot_var16;
		utf8_utf16_strcpy(&p, boot_var);
		ret = get_dp_device(boot_var16, &boot_dev);
		if (ret != EFI_SUCCESS)
			continue;

		if (device_is_present_and_system_part(boot_dev))
			break;

		efi_free_pool(boot_dev);
		boot_dev = NULL;
	}
out:
	if (boot_dev) {
		u16 *path_str;

		path_str = efi_dp_str(boot_dev);
		EFI_PRINT("EFI Capsule: bootdev is %ls\n", path_str);
		efi_free_pool(path_str);

		volume = efi_fs_from_path(boot_dev);
		if (!volume)
			ret = EFI_DEVICE_ERROR;
		else
			ret = EFI_CALL(volume->open_volume(volume,
							   &bootdev_root));
		efi_free_pool(boot_dev);
	} else {
		ret = EFI_NOT_FOUND;
	}
	free(boot_order);

	return ret;
}

/**
 * efi_capsule_scan_dir - traverse a capsule directory in boot device
 * @files:	Array of file names
 * @num:	Number of elements in @files
 *
 * Traverse a capsule directory in boot device.
 * Called by initialization code, and returns an array of capsule file
 * names in @files.
 *
 * Return:	status code
 */
static efi_status_t efi_capsule_scan_dir(u16 ***files, unsigned int *num)
{
	struct efi_file_handle *dirh;
	struct efi_file_info *dirent;
	efi_uintn_t dirent_size, tmp_size;
	unsigned int count;
	u16 **tmp_files;
	efi_status_t ret;

	ret = find_boot_device();
	if (ret == EFI_NOT_FOUND) {
		EFI_PRINT("EFI Capsule: bootdev is not set\n");
		*num = 0;
		return EFI_SUCCESS;
	} else if (ret != EFI_SUCCESS) {
		return EFI_DEVICE_ERROR;
	}

	/* count capsule files */
	ret = EFI_CALL((*bootdev_root->open)(bootdev_root, &dirh,
					     EFI_CAPSULE_DIR,
					     EFI_FILE_MODE_READ, 0));
	if (ret != EFI_SUCCESS) {
		*num = 0;
		return EFI_SUCCESS;
	}

	dirent_size = 256;
	dirent = malloc(dirent_size);
	if (!dirent)
		return EFI_OUT_OF_RESOURCES;

	count = 0;
	while (1) {
		tmp_size = dirent_size;
		ret = EFI_CALL((*dirh->read)(dirh, &tmp_size, dirent));
		if (ret == EFI_BUFFER_TOO_SMALL) {
			dirent = realloc(dirent, tmp_size);
			if (!dirent) {
				ret = EFI_OUT_OF_RESOURCES;
				goto err;
			}
			dirent_size = tmp_size;
			ret = EFI_CALL((*dirh->read)(dirh, &tmp_size, dirent));
		}
		if (ret != EFI_SUCCESS)
			goto err;
		if (!tmp_size)
			break;

		if (!(dirent->attribute & EFI_FILE_DIRECTORY) &&
		    u16_strcmp(dirent->file_name, L".") &&
		    u16_strcmp(dirent->file_name, L".."))
			count++;
	}

	ret = EFI_CALL((*dirh->setpos)(dirh, 0));
	if (ret != EFI_SUCCESS)
		goto err;

	/* make a list */
	tmp_files = malloc(count * sizeof(*files));
	if (!tmp_files) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	count = 0;
	while (1) {
		tmp_size = dirent_size;
		ret = EFI_CALL((*dirh->read)(dirh, &tmp_size, dirent));
		if (ret != EFI_SUCCESS)
			goto err;
		if (!tmp_size)
			break;

		if (!(dirent->attribute & EFI_FILE_DIRECTORY) &&
		    u16_strcmp(dirent->file_name, L".") &&
		    u16_strcmp(dirent->file_name, L".."))
			tmp_files[count++] = u16_strdup(dirent->file_name);
	}
	/* ignore an error */
	EFI_CALL((*dirh->close)(dirh));

	/* in ascii order */
	/* FIXME: u16 version of strcasecmp */
	qsort(tmp_files, count, sizeof(*tmp_files),
	      (int (*)(const void *, const void *))strcasecmp);
	*files = tmp_files;
	*num = count;
	ret = EFI_SUCCESS;
err:
	free(dirent);

	return ret;
}

/**
 * efi_capsule_read_file - read in a capsule file
 * @filename:	File name
 * @capsule:	Pointer to buffer for capsule
 *
 * Read a capsule file and put its content in @capsule.
 *
 * Return:	status code
 */
static efi_status_t efi_capsule_read_file(const u16 *filename,
					  struct efi_capsule_header **capsule)
{
	struct efi_file_handle *dirh, *fh;
	struct efi_file_info *file_info = NULL;
	struct efi_capsule_header *buf = NULL;
	efi_uintn_t size;
	efi_status_t ret;

	ret = EFI_CALL((*bootdev_root->open)(bootdev_root, &dirh,
					     EFI_CAPSULE_DIR,
					     EFI_FILE_MODE_READ, 0));
	if (ret != EFI_SUCCESS)
		return ret;
	ret = EFI_CALL((*dirh->open)(dirh, &fh, (u16 *)filename,
				     EFI_FILE_MODE_READ, 0));
	/* ignore an error */
	EFI_CALL((*dirh->close)(dirh));
	if (ret != EFI_SUCCESS)
		return ret;

	/* file size */
	size = 0;
	ret = EFI_CALL((*fh->getinfo)(fh, &efi_file_info_guid,
				      &size, file_info));
	if (ret == EFI_BUFFER_TOO_SMALL) {
		file_info = malloc(size);
		if (!file_info) {
			ret = EFI_OUT_OF_RESOURCES;
			goto err;
		}
		ret = EFI_CALL((*fh->getinfo)(fh, &efi_file_info_guid,
					      &size, file_info));
	}
	if (ret != EFI_SUCCESS)
		goto err;
	size = file_info->file_size;
	free(file_info);
	buf = malloc(size);
	if (!buf) {
		ret = EFI_OUT_OF_RESOURCES;
		goto err;
	}

	/* fetch data */
	ret = EFI_CALL((*fh->read)(fh, &size, buf));
	if (ret == EFI_SUCCESS) {
		if (size >= buf->capsule_image_size) {
			*capsule = buf;
		} else {
			free(buf);
			ret = EFI_INVALID_PARAMETER;
		}
	} else {
		free(buf);
	}
err:
	EFI_CALL((*fh->close)(fh));

	return ret;
}

/**
 * efi_capsule_delete_file - delete a capsule file
 * @filename:	File name
 *
 * Delete a capsule file from capsule directory.
 *
 * Return:	status code
 */
static efi_status_t efi_capsule_delete_file(const u16 *filename)
{
	struct efi_file_handle *dirh, *fh;
	efi_status_t ret;

	ret = EFI_CALL((*bootdev_root->open)(bootdev_root, &dirh,
					     EFI_CAPSULE_DIR,
					     EFI_FILE_MODE_READ, 0));
	if (ret != EFI_SUCCESS)
		return ret;
	ret = EFI_CALL((*dirh->open)(dirh, &fh, (u16 *)filename,
				     EFI_FILE_MODE_READ, 0));
	/* ignore an error */
	EFI_CALL((*dirh->close)(dirh));

	ret = EFI_CALL((*fh->delete)(fh));

	return ret;
}

/**
 * efi_capsule_scan_done - reset a scan help function
 *
 * Reset a scan help function
 */
static void efi_capsule_scan_done(void)
{
	EFI_CALL((*bootdev_root->close)(bootdev_root));
	bootdev_root = NULL;
}

/**
 * arch_efi_load_capsule_drivers - initialize capsule drivers
 *
 * Architecture or board specific initialization routine
 *
 * Return:	status code
 */
efi_status_t __weak arch_efi_load_capsule_drivers(void)
{
	return EFI_SUCCESS;
}

/**
 * efi_launch_capsule - launch capsules
 *
 * Launch all the capsules in system at boot time.
 * Called by efi init code
 *
 * Return:	status codde
 */
efi_status_t efi_launch_capsules(void)
{
	u64 os_indications;
	efi_uintn_t size;
	struct efi_capsule_header *capsule = NULL;
	u16 **files;
	unsigned int nfiles, index, i;
	u16 variable_name16[12];
	efi_status_t ret;

	size = sizeof(os_indications);
	ret = efi_get_variable_int(L"OsIndications", &efi_global_variable_guid,
				   NULL, &size, &os_indications, NULL);
	if (ret != EFI_SUCCESS ||
	    !(os_indications
	      & EFI_OS_INDICATIONS_FILE_CAPSULE_DELIVERY_SUPPORTED))
		return EFI_SUCCESS;

	index = get_last_capsule();

	/* Load capsule drivers */
	ret = arch_efi_load_capsule_drivers();
	if (ret != EFI_SUCCESS)
		return ret;

	/*
	 * Find capsules on disk.
	 * All the capsules are collected at the beginning because
	 * capsule files will be removed instantly.
	 */
	nfiles = 0;
	files = NULL;
	ret = efi_capsule_scan_dir(&files, &nfiles);
	if (ret != EFI_SUCCESS)
		return ret;
	if (!nfiles)
		return EFI_SUCCESS;

	/* Launch capsules */
	for (i = 0, ++index; i < nfiles; i++, index++) {
		EFI_PRINT("capsule from %ls ...\n", files[i]);
		if (index > 0xffff)
			index = 0;
		ret = efi_capsule_read_file(files[i], &capsule);
		if (ret == EFI_SUCCESS) {
			ret = EFI_CALL(efi_update_capsule(&capsule, 1, 0));
			if (ret != EFI_SUCCESS)
				printf("EFI Capsule update failed at %ls\n",
				       files[i]);

			free(capsule);
		} else {
			printf("EFI: reading capsule failed: %ls\n",
			       files[i]);
		}
		/* create CapsuleXXXX */
		set_capsule_result(index, capsule, ret);

		/* delete a capsule either in case of success or failure */
		ret = efi_capsule_delete_file(files[i]);
		if (ret != EFI_SUCCESS)
			printf("EFI: deleting a capsule file failed: %ls\n",
			       files[i]);
	}
	efi_capsule_scan_done();

	for (i = 0; i < nfiles; i++)
		free(files[i]);
	free(files);

	/* CapsuleLast */
	efi_create_indexed_name(variable_name16, "Capsule", index - 1);
	efi_set_variable_int(L"CapsuleLast", &efi_guid_capsule_report,
			     EFI_VARIABLE_READ_ONLY |
			     EFI_VARIABLE_NON_VOLATILE |
			     EFI_VARIABLE_BOOTSERVICE_ACCESS |
			     EFI_VARIABLE_RUNTIME_ACCESS,
			     22, variable_name16, false);

	return ret;
}
#endif /* CONFIG_EFI_CAPSULE_ON_DISK */

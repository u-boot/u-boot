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

#include <crypto/pkcs7.h>
#include <crypto/pkcs7_parser.h>
#include <linux/err.h>

const efi_guid_t efi_guid_capsule_report = EFI_CAPSULE_REPORT_GUID;
static const efi_guid_t efi_guid_firmware_management_capsule_id =
		EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;
const efi_guid_t efi_guid_firmware_management_protocol =
		EFI_FIRMWARE_MANAGEMENT_PROTOCOL_GUID;

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
	char value[5];
	efi_uintn_t size;
	unsigned long index = 0xffff;
	efi_status_t ret;
	int i;

	size = sizeof(value16);
	ret = efi_get_variable_int(L"CapsuleLast", &efi_guid_capsule_report,
				   NULL, &size, value16, NULL);
	if (ret != EFI_SUCCESS || size != 22 ||
	    u16_strncmp(value16, L"Capsule", 7))
		goto err;
	for (i = 0; i < 4; ++i) {
		u16 c = value16[i + 7];

		if (!c || c > 0x7f)
			goto err;
		value[i] = c;
	}
	value[4] = 0;
	if (strict_strtoul(value, 16, &index))
		index = 0xffff;
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

	efi_create_indexed_name(variable_name16, sizeof(variable_name16),
				"Capsule", index);
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
		log_err("EFI: creating %ls failed\n", variable_name16);
}

#ifdef CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT
/**
 * efi_fmp_find - search for Firmware Management Protocol drivers
 * @image_type:		Image type guid
 * @instance:		Instance number
 * @handles:		Handles of FMP drivers
 * @no_handles:		Number of handles
 *
 * Search for Firmware Management Protocol drivers, matching the image
 * type, @image_type and the machine instance, @instance, from the list,
 * @handles.
 *
 * Return:
 * * Protocol instance	- on success
 * * NULL		- on failure
 */
static struct efi_firmware_management_protocol *
efi_fmp_find(efi_guid_t *image_type, u64 instance, efi_handle_t *handles,
	     efi_uintn_t no_handles)
{
	efi_handle_t *handle;
	struct efi_firmware_management_protocol *fmp;
	struct efi_firmware_image_descriptor *image_info, *desc;
	efi_uintn_t info_size, descriptor_size;
	u32 descriptor_version;
	u8 descriptor_count;
	u32 package_version;
	u16 *package_version_name;
	bool found = false;
	int i, j;
	efi_status_t ret;

	for (i = 0, handle = handles; i < no_handles; i++, handle++) {
		ret = EFI_CALL(efi_handle_protocol(
				*handle,
				&efi_guid_firmware_management_protocol,
				(void **)&fmp));
		if (ret != EFI_SUCCESS)
			continue;

		/* get device's image info */
		info_size = 0;
		image_info = NULL;
		descriptor_version = 0;
		descriptor_count = 0;
		descriptor_size = 0;
		package_version = 0;
		package_version_name = NULL;
		ret = EFI_CALL(fmp->get_image_info(fmp, &info_size,
						   image_info,
						   &descriptor_version,
						   &descriptor_count,
						   &descriptor_size,
						   &package_version,
						   &package_version_name));
		if (ret != EFI_BUFFER_TOO_SMALL)
			goto skip;

		image_info = malloc(info_size);
		if (!image_info)
			goto skip;

		ret = EFI_CALL(fmp->get_image_info(fmp, &info_size,
						   image_info,
						   &descriptor_version,
						   &descriptor_count,
						   &descriptor_size,
						   &package_version,
						   &package_version_name));
		if (ret != EFI_SUCCESS ||
		    descriptor_version != EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION)
			goto skip;

		/* matching */
		for (j = 0, desc = image_info; j < descriptor_count;
		     j++, desc = (void *)desc + descriptor_size) {
			log_debug("+++ desc[%d] index: %d, name: %ls\n",
				  j, desc->image_index, desc->image_id_name);
			if (!guidcmp(&desc->image_type_id, image_type) &&
			    (!instance ||
			     !desc->hardware_instance ||
			      desc->hardware_instance == instance))
				found = true;
		}

skip:
		efi_free_pool(package_version_name);
		free(image_info);
		EFI_CALL(efi_close_protocol(
				(efi_handle_t)fmp,
				&efi_guid_firmware_management_protocol,
				NULL, NULL));
		if (found)
			return fmp;
	}

	return NULL;
}

#if defined(CONFIG_EFI_CAPSULE_AUTHENTICATE)

const efi_guid_t efi_guid_capsule_root_cert_guid =
	EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;

__weak int efi_get_public_key_data(void **pkey, efi_uintn_t *pkey_len)
{
	/* The platform is supposed to provide
	 * a method for getting the public key
	 * stored in the form of efi signature
	 * list
	 */
	return 0;
}

efi_status_t efi_capsule_authenticate(const void *capsule, efi_uintn_t capsule_size,
				      void **image, efi_uintn_t *image_size)
{
	u8 *buf;
	int ret;
	void *fdt_pkey, *pkey;
	efi_uintn_t pkey_len;
	uint64_t monotonic_count;
	struct efi_signature_store *truststore;
	struct pkcs7_message *capsule_sig;
	struct efi_image_regions *regs;
	struct efi_firmware_image_authentication *auth_hdr;
	efi_status_t status;

	status = EFI_SECURITY_VIOLATION;
	capsule_sig = NULL;
	truststore = NULL;
	regs = NULL;

	/* Sanity checks */
	if (capsule == NULL || capsule_size == 0)
		goto out;

	auth_hdr = (struct efi_firmware_image_authentication *)capsule;
	if (capsule_size < sizeof(*auth_hdr))
		goto out;

	if (auth_hdr->auth_info.hdr.dwLength <=
	    offsetof(struct win_certificate_uefi_guid, cert_data))
		goto out;

	if (guidcmp(&auth_hdr->auth_info.cert_type, &efi_guid_cert_type_pkcs7))
		goto out;

	*image = (uint8_t *)capsule + sizeof(auth_hdr->monotonic_count) +
		auth_hdr->auth_info.hdr.dwLength;
	*image_size = capsule_size - auth_hdr->auth_info.hdr.dwLength -
		sizeof(auth_hdr->monotonic_count);
	memcpy(&monotonic_count, &auth_hdr->monotonic_count,
	       sizeof(monotonic_count));

	/* data to be digested */
	regs = calloc(sizeof(*regs) + sizeof(struct image_region) * 2, 1);
	if (!regs)
		goto out;

	regs->max = 2;
	efi_image_region_add(regs, (uint8_t *)*image,
			     (uint8_t *)*image + *image_size, 1);

	efi_image_region_add(regs, (uint8_t *)&monotonic_count,
			     (uint8_t *)&monotonic_count + sizeof(monotonic_count),
			     1);

	capsule_sig = efi_parse_pkcs7_header(auth_hdr->auth_info.cert_data,
					     auth_hdr->auth_info.hdr.dwLength
					     - sizeof(auth_hdr->auth_info),
					     &buf);
	if (IS_ERR(capsule_sig)) {
		debug("Parsing variable's pkcs7 header failed\n");
		capsule_sig = NULL;
		goto out;
	}

	ret = efi_get_public_key_data(&fdt_pkey, &pkey_len);
	if (ret < 0)
		goto out;

	pkey = malloc(pkey_len);
	if (!pkey)
		goto out;

	memcpy(pkey, fdt_pkey, pkey_len);
	truststore = efi_build_signature_store(pkey, pkey_len);
	if (!truststore)
		goto out;

	/* verify signature */
	if (efi_signature_verify(regs, capsule_sig, truststore, NULL)) {
		debug("Verified\n");
	} else {
		debug("Verifying variable's signature failed\n");
		goto out;
	}

	status = EFI_SUCCESS;

out:
	efi_sigstore_free(truststore);
	pkcs7_free_message(capsule_sig);
	free(regs);

	return status;
}
#else
efi_status_t efi_capsule_authenticate(const void *capsule, efi_uintn_t capsule_size,
				      void **image, efi_uintn_t *image_size)
{
	return EFI_UNSUPPORTED;
}
#endif /* CONFIG_EFI_CAPSULE_AUTHENTICATE */


/**
 * efi_capsule_update_firmware - update firmware from capsule
 * @capsule_data:	Capsule
 *
 * Update firmware, using a capsule, @capsule_data. Loading any FMP
 * drivers embedded in a capsule is not supported.
 *
 * Return:		status code
 */
static efi_status_t efi_capsule_update_firmware(
		struct efi_capsule_header *capsule_data)
{
	struct efi_firmware_management_capsule_header *capsule;
	struct efi_firmware_management_capsule_image_header *image;
	size_t capsule_size;
	void *image_binary, *vendor_code;
	efi_handle_t *handles;
	efi_uintn_t no_handles;
	int item;
	struct efi_firmware_management_protocol *fmp;
	u16 *abort_reason;
	efi_status_t ret = EFI_SUCCESS;

	/* sanity check */
	if (capsule_data->header_size < sizeof(*capsule) ||
	    capsule_data->header_size >= capsule_data->capsule_image_size)
		return EFI_INVALID_PARAMETER;

	capsule = (void *)capsule_data + capsule_data->header_size;
	capsule_size = capsule_data->capsule_image_size
			- capsule_data->header_size;

	if (capsule->version != 0x00000001)
		return EFI_UNSUPPORTED;

	handles = NULL;
	ret = EFI_CALL(efi_locate_handle_buffer(
			BY_PROTOCOL,
			&efi_guid_firmware_management_protocol,
			NULL, &no_handles, (efi_handle_t **)&handles));
	if (ret != EFI_SUCCESS)
		return EFI_UNSUPPORTED;

	/* Payload */
	for (item = capsule->embedded_driver_count;
	     item < capsule->embedded_driver_count
		    + capsule->payload_item_count; item++) {
		/* sanity check */
		if ((capsule->item_offset_list[item] + sizeof(*image)
				 >= capsule_size)) {
			log_err("EFI: A capsule has not enough data\n");
			ret = EFI_INVALID_PARAMETER;
			goto out;
		}

		image = (void *)capsule + capsule->item_offset_list[item];

		if (image->version != 0x00000003) {
			ret = EFI_UNSUPPORTED;
			goto out;
		}

		/* find a device for update firmware */
		/* TODO: should we pass index as well, or nothing but type? */
		fmp = efi_fmp_find(&image->update_image_type_id,
				   image->update_hardware_instance,
				   handles, no_handles);
		if (!fmp) {
			log_err("EFI Capsule: driver not found for firmware type: %pUl, hardware instance: %lld\n",
				&image->update_image_type_id,
				image->update_hardware_instance);
			ret = EFI_UNSUPPORTED;
			goto out;
		}

		/* do update */
		image_binary = (void *)image + sizeof(*image);
		vendor_code = image_binary + image->update_image_size;

		abort_reason = NULL;
		ret = EFI_CALL(fmp->set_image(fmp, image->update_image_index,
					      image_binary,
					      image->update_image_size,
					      vendor_code, NULL,
					      &abort_reason));
		if (ret != EFI_SUCCESS) {
			log_err("EFI Capsule: firmware update failed: %ls\n",
				abort_reason);
			efi_free_pool(abort_reason);
			goto out;
		}
	}

out:
	efi_free_pool(handles);

	return ret;
}
#else
static efi_status_t efi_capsule_update_firmware(
		struct efi_capsule_header *capsule_data)
{
	return EFI_UNSUPPORTED;
}
#endif /* CONFIG_EFI_CAPSULE_FIRMWARE_MANAGEMENT */

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

	EFI_ENTRY("%p, %zu, %llu\n", capsule_header_array, capsule_count,
		  scatter_gather_list);

	if (!capsule_count) {
		ret = EFI_INVALID_PARAMETER;
		goto out;
	}

	ret = EFI_SUCCESS;
	for (i = 0, capsule = *capsule_header_array; i < capsule_count;
	     i++, capsule = *(++capsule_header_array)) {
		/* sanity check */
		if (capsule->header_size < sizeof(*capsule) ||
		    capsule->capsule_image_size < sizeof(*capsule)) {
			log_err("EFI: A capsule has not enough data\n");
			continue;
		}

		log_debug("Capsule[%d] (guid:%pUl)\n",
			  i, &capsule->capsule_guid);
		if (!guidcmp(&capsule->capsule_guid,
			     &efi_guid_firmware_management_capsule_id)) {
			ret  = efi_capsule_update_firmware(capsule);
		} else {
			log_err("EFI: not support capsule type: %pUl\n",
				&capsule->capsule_guid);
			ret = EFI_UNSUPPORTED;
		}

		if (ret != EFI_SUCCESS)
			goto out;
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

	EFI_ENTRY("%p, %zu, %p, %p\n", capsule_header_array, capsule_count,
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
			log_err("BootNext must be 16-bit integer\n");
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
		log_debug("EFI Capsule: bootdev is %ls\n", path_str);
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
		log_debug("EFI Capsule: bootdev is not set\n");
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

		if (!(dirent->attribute & EFI_FILE_DIRECTORY))
			count++;
	}

	ret = EFI_CALL((*dirh->setpos)(dirh, 0));
	if (ret != EFI_SUCCESS)
		goto err;

	/* make a list */
	tmp_files = malloc(count * sizeof(*tmp_files));
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
	__maybe_unused efi_handle_t handle;
	efi_status_t ret = EFI_SUCCESS;

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_FIT)) {
		handle = NULL;
		ret = EFI_CALL(efi_install_multiple_protocol_interfaces(
				&handle, &efi_guid_firmware_management_protocol,
				&efi_fmp_fit, NULL));
	}

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_FIRMWARE_RAW)) {
		handle = NULL;
		ret = EFI_CALL(efi_install_multiple_protocol_interfaces(
				&efi_root,
				&efi_guid_firmware_management_protocol,
				&efi_fmp_raw, NULL));
	}

	return ret;
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
		log_debug("capsule from %ls ...\n", files[i]);
		if (index > 0xffff)
			index = 0;
		ret = efi_capsule_read_file(files[i], &capsule);
		if (ret == EFI_SUCCESS) {
			ret = EFI_CALL(efi_update_capsule(&capsule, 1, 0));
			if (ret != EFI_SUCCESS)
				log_err("EFI Capsule update failed at %ls\n",
					files[i]);

			free(capsule);
		} else {
			log_err("EFI: reading capsule failed: %ls\n", files[i]);
		}
		/* create CapsuleXXXX */
		set_capsule_result(index, capsule, ret);

		/* delete a capsule either in case of success or failure */
		ret = efi_capsule_delete_file(files[i]);
		if (ret != EFI_SUCCESS)
			log_err("EFI: deleting a capsule file failed: %ls\n",
				files[i]);
	}
	efi_capsule_scan_done();

	for (i = 0; i < nfiles; i++)
		free(files[i]);
	free(files);

	/* CapsuleLast */
	efi_create_indexed_name(variable_name16, sizeof(variable_name16),
				"Capsule", index - 1);
	efi_set_variable_int(L"CapsuleLast", &efi_guid_capsule_report,
			     EFI_VARIABLE_READ_ONLY |
			     EFI_VARIABLE_NON_VOLATILE |
			     EFI_VARIABLE_BOOTSERVICE_ACCESS |
			     EFI_VARIABLE_RUNTIME_ACCESS,
			     22, variable_name16, false);

	return ret;
}
#endif /* CONFIG_EFI_CAPSULE_ON_DISK */

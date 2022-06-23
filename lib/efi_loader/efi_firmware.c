// SPDX-License-Identifier: GPL-2.0+
/*
 * EFI Firmware management protocol
 *
 *  Copyright (c) 2020 Linaro Limited
 *			Author: AKASHI Takahiro
 */

#include <common.h>
#include <charset.h>
#include <dfu.h>
#include <efi_loader.h>
#include <fwu.h>
#include <image.h>
#include <signatures.h>

#include <linux/list.h>

#define FMP_PAYLOAD_HDR_SIGNATURE	SIGNATURE_32('M', 'S', 'S', '1')

/**
 * struct fmp_payload_header - EDK2 header for the FMP payload
 *
 * This structure describes the header which is preprended to the
 * FMP payload by the edk2 capsule generation scripts.
 *
 * @signature:			Header signature used to identify the header
 * @header_size:		Size of the structure
 * @fw_version:			Firmware versions used
 * @lowest_supported_version:	Lowest supported version
 */
struct fmp_payload_header {
	u32 signature;
	u32 header_size;
	u32 fw_version;
	u32 lowest_supported_version;
};

__weak void set_dfu_alt_info(char *interface, char *devstr)
{
	env_set("dfu_alt_info", update_info.dfu_string);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_get_image_unsupported(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	void *image,
	efi_uintn_t *image_size)
{
	EFI_ENTRY("%p %d %p %p\n", this, image_index, image, image_size);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_check_image_unsupported(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	const void *image,
	efi_uintn_t *image_size,
	u32 *image_updatable)
{
	EFI_ENTRY("%p %d %p %p %p\n", this, image_index, image, image_size,
		  image_updatable);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_get_package_info_unsupported(
	struct efi_firmware_management_protocol *this,
	u32 *package_version,
	u16 **package_version_name,
	u32 *package_version_name_maxlen,
	u64 *attributes_supported,
	u64 *attributes_setting)
{
	EFI_ENTRY("%p %p %p %p %p %p\n", this, package_version,
		  package_version_name, package_version_name_maxlen,
		  attributes_supported, attributes_setting);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/* Place holder; not supported */
static
efi_status_t EFIAPI efi_firmware_set_package_info_unsupported(
	struct efi_firmware_management_protocol *this,
	const void *image,
	efi_uintn_t *image_size,
	const void *vendor_code,
	u32 package_version,
	const u16 *package_version_name)
{
	EFI_ENTRY("%p %p %p %p %x %p\n", this, image, image_size, vendor_code,
		  package_version, package_version_name);

	return EFI_EXIT(EFI_UNSUPPORTED);
}

/**
 * efi_fill_image_desc_array - populate image descriptor array
 * @image_info_size:		Size of @image_info
 * @image_info:			Image information
 * @descriptor_version:		Pointer to version number
 * @descriptor_count:		Image count
 * @descriptor_size:		Pointer to descriptor size
 * @package_version:		Package version
 * @package_version_name:	Package version's name
 *
 * Return information about the current firmware image in @image_info.
 * @image_info will consist of a number of descriptors.
 * Each descriptor will be created based on efi_fw_image array.
 *
 * Return		status code
 */
static efi_status_t efi_fill_image_desc_array(
	efi_uintn_t *image_info_size,
	struct efi_firmware_image_descriptor *image_info,
	u32 *descriptor_version,
	u8 *descriptor_count,
	efi_uintn_t *descriptor_size,
	u32 *package_version,
	u16 **package_version_name)
{
	size_t total_size;
	struct efi_fw_image *fw_array;
	int i;

	total_size = sizeof(*image_info) * num_image_type_guids;

	if (*image_info_size < total_size) {
		*image_info_size = total_size;

		return EFI_BUFFER_TOO_SMALL;
	}
	*image_info_size = total_size;

	fw_array = update_info.images;
	*descriptor_count = num_image_type_guids;
	*descriptor_version = EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION;
	*descriptor_size = sizeof(*image_info);
	*package_version = 0xffffffff; /* not supported */
	*package_version_name = NULL; /* not supported */

	for (i = 0; i < num_image_type_guids; i++) {
		image_info[i].image_index = fw_array[i].image_index;
		image_info[i].image_type_id = fw_array[i].image_type_id;
		image_info[i].image_id = fw_array[i].image_index;

		image_info[i].image_id_name = fw_array[i].fw_name;

		image_info[i].version = 0; /* not supported */
		image_info[i].version_name = NULL; /* not supported */
		image_info[i].size = 0;
		image_info[i].attributes_supported =
			IMAGE_ATTRIBUTE_IMAGE_UPDATABLE |
			IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED;
		image_info[i].attributes_setting =
				IMAGE_ATTRIBUTE_IMAGE_UPDATABLE;

		/* Check if the capsule authentication is enabled */
		if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE))
			image_info[0].attributes_setting |=
				IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED;

		image_info[i].lowest_supported_image_version = 0;
		image_info[i].last_attempt_version = 0;
		image_info[i].last_attempt_status = LAST_ATTEMPT_STATUS_SUCCESS;
		image_info[i].hardware_instance = 1;
		image_info[i].dependencies = NULL;
	}

	return EFI_SUCCESS;
}

/**
 * efi_firmware_capsule_authenticate - authenticate the capsule if enabled
 * @p_image:		Pointer to new image
 * @p_image_size:	Pointer to size of new image
 *
 * Authenticate the capsule if authentication is enabled.
 * The image pointer and the image size are updated in case of success.
 *
 * Return:		status code
 */
static
efi_status_t efi_firmware_capsule_authenticate(const void **p_image,
					       efi_uintn_t *p_image_size)
{
	const void *image = *p_image;
	efi_uintn_t image_size = *p_image_size;
	u32 fmp_hdr_signature;
	struct fmp_payload_header *header;
	void *capsule_payload;
	efi_status_t status;
	efi_uintn_t capsule_payload_size;

	if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)) {
		capsule_payload = NULL;
		capsule_payload_size = 0;
		status = efi_capsule_authenticate(image, image_size,
						  &capsule_payload,
						  &capsule_payload_size);

		if (status == EFI_SECURITY_VIOLATION) {
			printf("Capsule authentication check failed. Aborting update\n");
			return status;
		} else if (status != EFI_SUCCESS) {
			return status;
		}

		debug("Capsule authentication successful\n");
		image = capsule_payload;
		image_size = capsule_payload_size;
	} else {
		debug("Capsule authentication disabled. ");
		debug("Updating capsule without authenticating.\n");
	}

	fmp_hdr_signature = FMP_PAYLOAD_HDR_SIGNATURE;
	header = (void *)image;

	if (!memcmp(&header->signature, &fmp_hdr_signature,
		    sizeof(fmp_hdr_signature))) {
		/*
		 * When building the capsule with the scripts in
		 * edk2, a FMP header is inserted above the capsule
		 * payload. Compensate for this header to get the
		 * actual payload that is to be updated.
		 */
		image += header->header_size;
		image_size -= header->header_size;
	}

	*p_image = image;
	*p_image_size = image_size;
	return EFI_SUCCESS;
}

/**
 * efi_firmware_get_image_info - return information about the current
 *				     firmware image
 * @this:			Protocol instance
 * @image_info_size:		Size of @image_info
 * @image_info:			Image information
 * @descriptor_version:		Pointer to version number
 * @descriptor_count:		Pointer to number of descriptors
 * @descriptor_size:		Pointer to descriptor size
 * @package_version:		Package version
 * @package_version_name:	Package version's name
 *
 * Return information bout the current firmware image in @image_info.
 * @image_info will consist of a number of descriptors.
 * Each descriptor will be created based on "dfu_alt_info" variable.
 *
 * Return		status code
 */
static
efi_status_t EFIAPI efi_firmware_get_image_info(
	struct efi_firmware_management_protocol *this,
	efi_uintn_t *image_info_size,
	struct efi_firmware_image_descriptor *image_info,
	u32 *descriptor_version,
	u8 *descriptor_count,
	efi_uintn_t *descriptor_size,
	u32 *package_version,
	u16 **package_version_name)
{
	efi_status_t ret;

	EFI_ENTRY("%p %p %p %p %p %p %p %p\n", this,
		  image_info_size, image_info,
		  descriptor_version, descriptor_count, descriptor_size,
		  package_version, package_version_name);

	if (!image_info_size)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	if (*image_info_size &&
	    (!image_info || !descriptor_version || !descriptor_count ||
	     !descriptor_size || !package_version || !package_version_name))
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	ret = efi_fill_image_desc_array(image_info_size, image_info,
					descriptor_version, descriptor_count,
					descriptor_size, package_version,
					package_version_name);

	return EFI_EXIT(ret);
}

#ifdef CONFIG_EFI_CAPSULE_FIRMWARE_FIT
/*
 * This FIRMWARE_MANAGEMENT_PROTOCOL driver provides a firmware update
 * method with existing FIT image format, and handles
 *   - multiple regions of firmware via DFU
 * but doesn't support
 *   - versioning of firmware image
 *   - package information
 */

/**
 * efi_firmware_fit_set_image - update the firmware image
 * @this:		Protocol instance
 * @image_index:	Image index number
 * @image:		New image
 * @image_size:		Size of new image
 * @vendor_code:	Vendor-specific update policy
 * @progress:		Function to report the progress of update
 * @abort_reason:	Pointer to string of abort reason
 *
 * Update the firmware to new image, using dfu. The new image should
 * have FIT image format commonly used in U-Boot.
 * @vendor_code, @progress and @abort_reason are not supported.
 *
 * Return:		status code
 */
static
efi_status_t EFIAPI efi_firmware_fit_set_image(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	const void *image,
	efi_uintn_t image_size,
	const void *vendor_code,
	efi_status_t (*progress)(efi_uintn_t completion),
	u16 **abort_reason)
{
	efi_status_t status;

	EFI_ENTRY("%p %d %p %zu %p %p %p\n", this, image_index, image,
		  image_size, vendor_code, progress, abort_reason);

	if (!image || image_index != 1)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	status = efi_firmware_capsule_authenticate(&image, &image_size);
	if (status != EFI_SUCCESS)
		return EFI_EXIT(status);

	if (fit_update(image))
		return EFI_EXIT(EFI_DEVICE_ERROR);

	return EFI_EXIT(EFI_SUCCESS);
}

const struct efi_firmware_management_protocol efi_fmp_fit = {
	.get_image_info = efi_firmware_get_image_info,
	.get_image = efi_firmware_get_image_unsupported,
	.set_image = efi_firmware_fit_set_image,
	.check_image = efi_firmware_check_image_unsupported,
	.get_package_info = efi_firmware_get_package_info_unsupported,
	.set_package_info = efi_firmware_set_package_info_unsupported,
};
#endif /* CONFIG_EFI_CAPSULE_FIRMWARE_FIT */

#ifdef CONFIG_EFI_CAPSULE_FIRMWARE_RAW
/*
 * This FIRMWARE_MANAGEMENT_PROTOCOL driver provides a firmware update
 * method with raw data.
 */

/**
 * efi_firmware_raw_set_image - update the firmware image
 * @this:		Protocol instance
 * @image_index:	Image index number
 * @image:		New image
 * @image_size:		Size of new image
 * @vendor_code:	Vendor-specific update policy
 * @progress:		Function to report the progress of update
 * @abort_reason:	Pointer to string of abort reason
 *
 * Update the firmware to new image, using dfu. The new image should
 * be a single raw image.
 * @vendor_code, @progress and @abort_reason are not supported.
 *
 * Return:		status code
 */
static
efi_status_t EFIAPI efi_firmware_raw_set_image(
	struct efi_firmware_management_protocol *this,
	u8 image_index,
	const void *image,
	efi_uintn_t image_size,
	const void *vendor_code,
	efi_status_t (*progress)(efi_uintn_t completion),
	u16 **abort_reason)
{
	int ret;
	efi_status_t status;

	EFI_ENTRY("%p %d %p %zu %p %p %p\n", this, image_index, image,
		  image_size, vendor_code, progress, abort_reason);

	if (!image)
		return EFI_EXIT(EFI_INVALID_PARAMETER);

	status = efi_firmware_capsule_authenticate(&image, &image_size);
	if (status != EFI_SUCCESS)
		return EFI_EXIT(status);

	if (IS_ENABLED(CONFIG_FWU_MULTI_BANK_UPDATE)) {
		/*
		 * Based on the value of update bank, derive the
		 * image index value.
		 */
		ret = fwu_get_image_index(&image_index);
		if (ret) {
			log_debug("Unable to get FWU image_index\n");
			return EFI_EXIT(EFI_DEVICE_ERROR);
		}
	}

	if (dfu_write_by_alt(image_index - 1, (void *)image, image_size,
			     NULL, NULL))
		return EFI_EXIT(EFI_DEVICE_ERROR);

	return EFI_EXIT(EFI_SUCCESS);
}

const struct efi_firmware_management_protocol efi_fmp_raw = {
	.get_image_info = efi_firmware_get_image_info,
	.get_image = efi_firmware_get_image_unsupported,
	.set_image = efi_firmware_raw_set_image,
	.check_image = efi_firmware_check_image_unsupported,
	.get_package_info = efi_firmware_get_package_info_unsupported,
	.set_package_info = efi_firmware_set_package_info_unsupported,
};
#endif /* CONFIG_EFI_CAPSULE_FIRMWARE_RAW */

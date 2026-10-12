// SPDX-License-Identifier: GPL-2.0-only
/*
 * Test Firmware Management Protocol support.
 *
 * Copyright (C) 2026 Arm Ltd.
 */
#include <efi_selftest.h>

static const efi_guid_t firmware_management_protocol_guid =
	EFI_FIRMWARE_MANAGEMENT_PROTOCOL_GUID;

static struct efi_boot_services *boottime;

/**
 * setup() - Setup unit test.
 * @handle:	handle of the loaded image
 * @systable:	system table
 *
 * Keep Boot Services.
 *
 * Return:	EFI_ST_SUCCESS for success.
 */
static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;
	return EFI_ST_SUCCESS;
}

/**
 * test_fmp() - Perform FMP test on one handle.
 * @handle:	firmware management protocol handle to test
 *
 * Call GetImageInfo() to retrieve current firmware images descriptors.
 * For each descriptor, verify image authentication consistency.
 *
 * Return:	EFI_ST_SUCCESS for success, EFI_ST_FAILURE in case of error.
 */
static int test_fmp(efi_handle_t handle)
{
	efi_status_t ret;
	efi_uintn_t img_info_size, desc_size, total_size;
	struct efi_firmware_image_descriptor *img_info = NULL;
	u32 desc_ver, pkg_ver;
	u8 desc_cnt;
	u16 *pkg_ver_name = NULL;
	int i, result = EFI_ST_FAILURE;
	struct efi_firmware_management_protocol *fmp;

	/* Get the FMP of this handle */
	ret = boottime->handle_protocol(handle,
					&firmware_management_protocol_guid,
					(void **)&fmp);
	if (ret != EFI_SUCCESS) {
		efi_st_error("HandleProtocol returned %u\n", (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	/* Determine buffer size */
	img_info_size = 0;
	ret = fmp->get_image_info(fmp, &img_info_size, NULL, NULL, NULL, NULL,
				  NULL, NULL);
	if (ret != EFI_BUFFER_TOO_SMALL) {
		efi_st_error("GetImageInfo returned %u\n", (unsigned int)ret);
		goto out;
	}

	/* Allocate enough memory for image info */
	ret = boottime->allocate_pool(EFI_LOADER_DATA, img_info_size,
				      (void **)&img_info);
	if (ret != EFI_SUCCESS) {
		efi_st_error("AllocatePool returned %u\n", (unsigned int)ret);
		goto out;
	}

	ret = fmp->get_image_info(fmp, &img_info_size, img_info, &desc_ver,
				  &desc_cnt, &desc_size, &pkg_ver,
				  &pkg_ver_name);
	if (ret != EFI_SUCCESS) {
		efi_st_error("GetImageInfo returned %u\n", (unsigned int)ret);
		goto out;
	}

	if (desc_ver != EFI_FIRMWARE_IMAGE_DESCRIPTOR_VERSION) {
		efi_st_error("Bad DescriptorVersion %u\n", desc_ver);
		goto out;
	}

	if (desc_size < sizeof(struct efi_firmware_image_descriptor) ||
	    __builtin_mul_overflow(desc_size, desc_cnt, &total_size) ||
	    total_size > img_info_size) {
		efi_st_error("Bad DescriptorSize %u\n",
			     (unsigned int)desc_size);
		goto out;
	}

	/* Verify each image info */
	for (i = 0; i < desc_cnt; i++) {
		const struct efi_firmware_image_descriptor *inf =
			(const struct efi_firmware_image_descriptor *)
			((uintptr_t)img_info + i * desc_size);
		u64 attr_supp = inf->attributes_supported,
		    attr_set = inf->attributes_setting;
		const u64 attr_mask = IMAGE_ATTRIBUTE_IMAGE_UPDATABLE |
				      IMAGE_ATTRIBUTE_RESET_REQUIRED |
				      IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED |
				      IMAGE_ATTRIBUTE_IN_USE |
				      IMAGE_ATTRIBUTE_UEFI_IMAGE |
				      IMAGE_ATTRIBUTE_DEPENDENCY;

		efi_st_printf("  handle %p, image %u: %ps\n", handle,
			      inf->image_index, inf->image_id_name);

		/* No unknown image attributes supported or set. */
		if (attr_supp & ~attr_mask) {
			efi_st_error("Unknown AttributesSupported %u\n",
				     (unsigned int)attr_supp);
			goto out;
		}

		if (attr_set & ~attr_mask) {
			efi_st_error("Unknown AttributesSetting %u\n",
				     (unsigned int)attr_set);
			goto out;
		}

		/* No unsupported image attributes set. */
		if (attr_set & ~attr_supp) {
			efi_st_error("Unsupported AttributesSetting %u\n",
				     (unsigned int)attr_set);
			goto out;
		}

		/*
		 * When support for authentication is compiled in, our updatable
		 * images must require it, both as supported and set.
		 */
		if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE) &&
		    (attr_set & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE) &&
		    !(attr_supp & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED)) {
			efi_st_error("Bad AttributesSupported %u\n",
				     (unsigned int)attr_supp);
			goto out;
		}

		if (IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE) &&
		    (attr_set & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE) &&
		    !(attr_set & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED)) {
			efi_st_error("Bad AttributesSetting %u\n",
				     (unsigned int)attr_set);
			goto out;
		}

		/*
		 * When support for authentication is not compiled in, our
		 * updatable images must not advertise it as set.
		 */
		if (!IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE) &&
		    (attr_set & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE) &&
		    (attr_set & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED)) {
			efi_st_error("Bad AttributesSetting %u\n",
				     (unsigned int)attr_set);
			goto out;
		}
	}

	result = EFI_ST_SUCCESS;

out:
	if (img_info) {
		ret = boottime->free_pool(img_info);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool returned %u\n",
				     (unsigned int)ret);
			result = EFI_ST_FAILURE;
		}
	}

	if (pkg_ver_name) {
		ret = boottime->free_pool(pkg_ver_name);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool returned %u\n",
				     (unsigned int)ret);
			result = EFI_ST_FAILURE;
		}
	}

	return result;
}

/**
 * execute() - Perform the test on all handles with FMP.
 *
 * Call test_fmp() on each handle in the system supporting FMP.
 *
 * Return:	EFI_ST_SUCCESS for success, EFI_ST_FAILURE in case of error.
 */
static int execute(void)
{
	int i, result = EFI_ST_SUCCESS;
	efi_handle_t *buffer = NULL;
	efi_uintn_t count;
	efi_status_t ret;

	/* Locate all handles with FMP */
	ret = boottime->locate_handle_buffer(BY_PROTOCOL,
					     &firmware_management_protocol_guid,
					     NULL, &count, &buffer);
	if (ret != EFI_SUCCESS) {
		efi_st_error("LocateHandleBuffer returned %u\n",
			     (unsigned int)ret);
		return EFI_ST_FAILURE;
	}

	/* Test all handles */
	for (i = 0; i < count; i++) {
		result = test_fmp(buffer[i]);
		if (result != EFI_ST_SUCCESS)
			break;
	}

	if (buffer) {
		/* Free handles buffer */
		ret = boottime->free_pool(buffer);
		if (ret != EFI_SUCCESS) {
			efi_st_error("FreePool returned %u\n",
				     (unsigned int)ret);
			result = EFI_ST_FAILURE;
		}
	}

	return result;
}

EFI_UNIT_TEST(fmp) = {
	.name = "firmware management protocol",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * efi_selftest_load_initrd
 *
 * Copyright (c) 2020 Ilias Apalodimas <ilias.apalodimas@linaro.org>
 *
 * This test checks the FileLoad2 protocol.
 * A known file is read from the file system and verified.
 *
 * An example usage - given a file image with a file system in partition 1
 * holding file initrd - is:
 *
 * * Configure the sandbox with
 *
 *   CONFIG_EFI_SELFTEST=y
 *   CONFIG_EFI_LOAD_FILE2_INITRD=y
 *   CONFIG_EFI_INITRD_FILESPEC="host 0:1 initrd"
 *
 * * Run ./u-boot and execute
 *
 *   host bind 0 image
 *   setenv efi_selftest load initrd
 *   bootefi selftest
 *
 * This would provide a test output like:
 *
 *   Testing EFI API implementation
 *
 *   Selected test: 'load initrd'
 *
 *   Setting up 'load initrd'
 *   Setting up 'load initrd' succeeded
 *
 *   Executing 'load initrd'
 *   Loaded 12378613 bytes
 *   CRC32 2997478465
 *
 * Now the size and CRC32 can be compared to the provided file.
 */

#include <efi_selftest.h>
#include <efi_loader.h>
#include <efi_load_initrd.h>

static struct efi_boot_services *boottime;

static struct efi_initrd_dp dp = {
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

static struct efi_initrd_dp dp_invalid = {
	.vendor = {
		{
		   DEVICE_PATH_TYPE_MEDIA_DEVICE,
		   DEVICE_PATH_SUB_TYPE_VENDOR_PATH,
		   sizeof(dp.vendor),
		},
		EFI_INITRD_MEDIA_GUID,
	},
	.end = {
		0x8f, /* invalid */
		0xfe, /* invalid */
		sizeof(dp.end),
	}
};

static int setup(const efi_handle_t handle,
		 const struct efi_system_table *systable)
{
	boottime = systable->boottime;

	return EFI_ST_SUCCESS;
}

static int execute(void)
{
	efi_guid_t lf2_proto_guid = EFI_LOAD_FILE2_PROTOCOL_GUID;
	struct efi_load_file_protocol *lf2;
	struct efi_device_path *dp2, *dp2_invalid;
	efi_status_t status;
	efi_handle_t handle;
	char buffer[64];
	efi_uintn_t buffer_size;
	void *buf;
	u32 crc32;

	memset(buffer, 0, sizeof(buffer));

	dp2 = (struct efi_device_path *)&dp;
	status = boottime->locate_device_path(&lf2_proto_guid, &dp2, &handle);
	if (status != EFI_SUCCESS) {
		efi_st_error("Unable to locate device path\n");
		return EFI_ST_FAILURE;
	}

	status = boottime->handle_protocol(handle, &lf2_proto_guid,
					   (void **)&lf2);
	if (status != EFI_SUCCESS) {
		efi_st_error("Unable to locate protocol\n");
		return EFI_ST_FAILURE;
	}

	/* Case 1:
	 * buffer_size can't be NULL
	 * protocol can't be NULL
	 */
	status = lf2->load_file(lf2, dp2, false, NULL, &buffer);
	if (status != EFI_INVALID_PARAMETER) {
		efi_st_error("Buffer size can't be NULL\n");
		return EFI_ST_FAILURE;
	}
	buffer_size = sizeof(buffer);
	status = lf2->load_file(NULL, dp2, false, &buffer_size, &buffer);
	if (status != EFI_INVALID_PARAMETER) {
		efi_st_error("Protocol can't be NULL\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Case 2: Match end node type/sub-type on device path
	 */
	dp2_invalid = (struct efi_device_path *)&dp_invalid;
	buffer_size = sizeof(buffer);
	status = lf2->load_file(lf2, dp2_invalid, false, &buffer_size, &buffer);
	if (status != EFI_INVALID_PARAMETER) {
		efi_st_error("Invalid device path type must return EFI_INVALID_PARAMETER\n");
		return EFI_ST_FAILURE;
	}

	status = lf2->load_file(lf2, dp2_invalid, false, &buffer_size, &buffer);
	if (status != EFI_INVALID_PARAMETER) {
		efi_st_error("Invalid device path sub-type must return EFI_INVALID_PARAMETER\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Case 3:
	 * BootPolicy 'true' must return EFI_UNSUPPORTED
	 */
	buffer_size = sizeof(buffer);
	status = lf2->load_file(lf2, dp2, true, &buffer_size, &buffer);
	if (status != EFI_UNSUPPORTED) {
		efi_st_error("BootPolicy true must return EFI_UNSUPPORTED\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Case: Pass buffer size as zero, firmware must return
	 * EFI_BUFFER_TOO_SMALL and an appropriate size
	 */
	buffer_size = 0;
	status = lf2->load_file(lf2, dp2, false, &buffer_size, NULL);
	if (status != EFI_BUFFER_TOO_SMALL || !buffer_size) {
		efi_st_printf("buffer_size: %u\n", (unsigned int)buffer_size);
		efi_st_printf("status: %x\n", (unsigned int)status);
		efi_st_error("Buffer size not updated\n");
		return EFI_ST_FAILURE;
	}

	/*
	 * Case: Pass buffer size as smaller than the file_size,
	 * firmware must return * EFI_BUFFER_TOO_SMALL and an appropriate size
	 */
	buffer_size = 1;
	status = lf2->load_file(lf2, dp2, false, &buffer_size, &buffer);
	if (status != EFI_BUFFER_TOO_SMALL || buffer_size <= 1) {
		efi_st_error("Buffer size not updated\n");
		return EFI_ST_FAILURE;
	}

	status = boottime->allocate_pool(EFI_BOOT_SERVICES_DATA, buffer_size,
					 &buf);
	if (status != EFI_SUCCESS) {
		efi_st_error("Cannot allocate buffer\n");
		return EFI_ST_FAILURE;
	}

	/* Case: Pass correct buffer, load the file and verify checksum*/
	status = lf2->load_file(lf2, dp2, false, &buffer_size, buf);
	if (status != EFI_SUCCESS) {
		efi_st_error("Loading initrd failed\n");
		return EFI_ST_FAILURE;
	}

	efi_st_printf("Loaded %u bytes\n", (unsigned int)buffer_size);
	status = boottime->calculate_crc32(buf, buffer_size, &crc32);
	if (status != EFI_SUCCESS) {
		efi_st_error("Could not determine CRC32\n");
		return EFI_ST_FAILURE;
	}
	efi_st_printf("CRC32 %.8x\n", (unsigned int)crc32);

	status = boottime->free_pool(buf);
	if (status != EFI_SUCCESS) {
		efi_st_error("Cannot free buffer\n");
		return EFI_ST_FAILURE;
	}

	return EFI_ST_SUCCESS;
}

EFI_UNIT_TEST(load_initrd) = {
	.name = "load initrd",
	.phase = EFI_EXECUTE_BEFORE_BOOTTIME_EXIT,
	.setup = setup,
	.execute = execute,
	.on_request = true,
};

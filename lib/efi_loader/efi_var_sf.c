// SPDX-License-Identifier: GPL-2.0+
/*
 * SPI Flash interface for UEFI variables
 *
 * Copyright (c) 2023, Shantur Rathore
 * Copyright (C) 2026, Advanced Micro Devices, Inc.
 */

#define LOG_CATEGORY LOGC_EFI

#include <efi_loader.h>
#include <efi_variable.h>
#include <spi_flash.h>
#include <dm.h>

efi_status_t efi_var_to_storage(void)
{
	struct efi_var_file *buf;
	struct spi_flash *flash;
	struct udevice *sfdev;
	efi_status_t ret;
	size_t erase_len;
	loff_t len;
	int r;

	ret = efi_var_collect(&buf, &len, EFI_VARIABLE_NON_VOLATILE);
	if (ret != EFI_SUCCESS)
		goto error;

	if (len > EFI_VAR_BUF_SIZE) {
		log_debug("EFI var buffer length more than target SPI Flash size\n");
		ret = EFI_OUT_OF_RESOURCES;
		goto error;
	}

	log_debug("Got buffer to write buf->len: %d\n", buf->length);

	r = uclass_get_device(UCLASS_SPI_FLASH,
			      CONFIG_EFI_VARIABLE_SF_DEVICE_INDEX, &sfdev);
	if (r) {
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	flash = dev_get_uclass_priv(sfdev);
	if (!flash) {
		log_debug("Failed to get SPI Flash priv data\n");
		ret = EFI_DEVICE_ERROR;
		goto error;
	}
	erase_len = ALIGN(len, flash->sector_size);

	r = spi_flash_erase_dm(sfdev, CONFIG_EFI_VARIABLE_SF_OFFSET,
			       erase_len);
	if (r) {
		log_debug("Failed to erase SPI Flash\n");
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	r = spi_flash_write_dm(sfdev, CONFIG_EFI_VARIABLE_SF_OFFSET, len, buf);
	if (r) {
		log_debug("Failed to write to SPI Flash: %d\n", r);
		ret = EFI_DEVICE_ERROR;
	}

error:
	free(buf);
	return ret;
}

efi_status_t efi_var_from_storage(void)
{
	struct efi_var_file *buf;
	struct udevice *sfdev;
	efi_status_t ret;
	int r;

	buf = calloc(1, EFI_VAR_BUF_SIZE);
	if (!buf) {
		log_err("Unable to allocate buffer\n");
		return EFI_OUT_OF_RESOURCES;
	}

	r = uclass_get_device(UCLASS_SPI_FLASH,
			      CONFIG_EFI_VARIABLE_SF_DEVICE_INDEX, &sfdev);
	if (r) {
		log_err("Failed to get SPI Flash device: %d\n", r);
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	r = spi_flash_read_dm(sfdev, CONFIG_EFI_VARIABLE_SF_OFFSET,
			      EFI_VAR_BUF_SIZE, buf);
	if (r) {
		log_err("Failed to read from SPI Flash: %d\n", r);
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	if (efi_var_restore(buf, false) != EFI_SUCCESS) {
		log_err("No valid EFI variables in SPI Flash\n");
		ret = EFI_DEVICE_ERROR;
		goto error;
	}

	ret = EFI_SUCCESS;
error:
	free(buf);
	return ret;
}

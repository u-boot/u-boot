/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 Collabora Ltd.
 */

#ifndef _FB_SPI_FLASH_H_
#define _FB_SPI_FLASH_H_

#include <part.h>

/**
 * fastboot_spi_flash_get_part_info() - Lookup SPI flash partition by name
 *
 * @part_name: Named device to lookup
 * @part_info: Pointer to returned struct disk_partition
 * @response: Pointer to fastboot response buffer
 * Return: 0 if OK, -ENOENT if no partition name was given, -ENODEV on invalid
 * raw partition descriptor
 */
int fastboot_spi_flash_get_part_info(const char *part_name,
				     struct disk_partition *part_info,
				     char *response);

/**
 * fastboot_spi_flash_write() - Write image to SPI flash for fastboot
 *
 * @cmd: Named device to write image to
 * @download_buffer: Pointer to image data
 * @download_bytes: Size of image data
 * @response: Pointer to fastboot response buffer
 */
void fastboot_spi_flash_write(const char *cmd, void *download_buffer,
			      u32 download_bytes, char *response);

/**
 * fastboot_spi_flash_erase() - Erase SPI flash for fastboot
 *
 * @cmd: Named device to erase
 * @response: Pointer to fastboot response buffer
 */
void fastboot_spi_flash_erase(const char *cmd, char *response);
#endif

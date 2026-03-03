/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2018-2019 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Lucien Jheng <lucienzx159@gmail.com>
 */
#ifndef _FW_LOADER_H_
#define _FW_LOADER_H_

#include <dm/ofnode_decl.h>

struct udevice;

/**
 * get_fw_loader_from_node - Get FW loader dev from @node.
 *
 * @node: ofnode where "firmware-loader" phandle is stored.
 * @dev: pointer where to store the FW loader dev.
 *
 * Loop over all the supported FW loader and find a matching
 * one.
 *
 * Return: Negative value if fail, 0 for successful.
 */
int get_fw_loader_from_node(ofnode node, struct udevice **dev);

/**
 * request_firmware_into_buf - Load firmware into a previously allocated buffer.
 * @dev: An instance of a driver.
 * @name: Name of firmware file.
 * @buf: Address of buffer to load firmware into.
 * @size: Size of buffer.
 * @offset: Offset of a file for start reading into buffer.
 *
 * The firmware is loaded directly into the buffer pointed to by @buf.
 *
 * Return: Size of total read, negative value when error.
 */
int request_firmware_into_buf(struct udevice *dev,
			      const char *name,
			      void *buf, size_t size, u32 offset);

/**
 * request_firmware_size - Get firmware size.
 * @dev: An instance of a driver.
 * @name: Name of firmware file.
 *
 * Return: Size of firmware, negative value when error.
 */
int request_firmware_size(struct udevice *dev, const char *name);

/**
 * request_firmware_into_buf_via_script() -
 * Load firmware using a U-Boot script and copy to buffer
 * @buf: Pointer to a pointer where the firmware buffer will be stored.
 * @max_size: Maximum allowed size for the firmware to be loaded.
 * @script_name: Name of the U-Boot script to execute for firmware loading.
 * @retsize: Return the actual firmware data size (optional).
 *
 * Executes a U-Boot script (@script_name) that loads firmware into
 * memory and sets the environment variables 'fw_addr' (address) and
 * 'fw_size' (size in bytes). On success, copies the firmware
 * from the given address to user buffer @buf.
 *
 * The script must set these environment variables:
 *   fw_addr - Address where firmware is loaded in memory
 *   fw_size - Size of the firmware in bytes
 *
 * The script should be defined in the U-Boot environment, for example:
 *   env set script_name 'load mmc 0:1 ${loadaddr} firmware.bin &&
 *   env set fw_addr ${loadaddr} && env set fw_size ${filesize}
 * Return: 0 on success, negative value on error.
 */
int request_firmware_into_buf_via_script(void *buf, size_t max_size,
					 const char *script_name,
					 size_t *retsize);

#endif

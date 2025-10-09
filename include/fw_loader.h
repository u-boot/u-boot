/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025 Lucien Jheng <lucienzx159@gmail.com>
 */
#ifndef _FW_LOADER_H_
#define _FW_LOADER_H_

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

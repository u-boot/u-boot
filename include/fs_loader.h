/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 *
 * SPDX-License-Identifier:    GPL-2.0
 */
#ifndef _FS_LOADER_H_
#define _FS_LOADER_H_

struct udevice;

/**
 * struct phandle_part - A place for storing phandle of node and its partition
 *
 * This holds information about a phandle of the block device, and its
 * partition where the firmware would be loaded from.
 *
 * @phandle: Phandle of storage device node
 * @partition: Partition of block device
 */
struct phandle_part {
	u32 phandle;
	u32 partition;
};

/**
 * struct phandle_part - A place for storing all supported storage devices
 *
 * This holds information about all supported storage devices for driver use.
 *
 * @phandlepart: Attribute data for block device.
 * @mtdpart: MTD partition for ubi partition.
 * @ubivol: UBI volume-name for ubifsmount.
 */
struct device_plat {
	struct phandle_part phandlepart;
	char *mtdpart;
	char *ubivol;
};

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
 * get_fs_loader() - Get the chosen filesystem loader
 * @dev: Where to store the device
 *
 * This gets a filesystem loader device based on the value of
 * /chosen/firmware-loader. If no such property exists, it returns a
 * firmware loader which is configured by environmental variables.
 *
 * Return: 0 on success, negative value on error
 */
int get_fs_loader(struct udevice **dev);

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
int request_firmware_into_buf_via_script(void **buf, size_t max_size,
					 const char *script_name,
					 size_t *retsize);
#endif

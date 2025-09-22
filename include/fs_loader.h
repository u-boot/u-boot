/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 *
 * SPDX-License-Identifier:    GPL-2.0
 */
#ifndef _FS_LOADER_H_
#define _FS_LOADER_H_

#include <fw_loader.h>

struct udevice;

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

#endif

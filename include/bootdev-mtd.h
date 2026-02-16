/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MTD and UBI boot device accessors
 *
 * Copyright (C) 2026 Daniel Golle <daniel@makrotopia.org>
 */

#ifndef __BOOTDEV_MTD_H
#define __BOOTDEV_MTD_H

#include <dm/device.h>

struct mtd_info;

/**
 * mtd_bootdev_get_mtd() - get the whole-flash MTD device for an mtd_bootdev
 *
 * @dev: mtd_bootdev udevice
 *
 * Return: pointer to the top-level struct mtd_info, or NULL
 */
struct mtd_info *mtd_bootdev_get_mtd(struct udevice *dev);

/**
 * ubi_bootdev_get_ubi_mtd() - get the MTD partition hosting UBI
 *
 * @dev: ubi_bootdev udevice
 *
 * Return: pointer to the MTD partition's struct mtd_info, or NULL
 */
struct mtd_info *ubi_bootdev_get_ubi_mtd(struct udevice *dev);

/**
 * bootdev_get_mtd() - get the backing MTD device for any MTD-based bootdev
 *
 * Dispatches to the appropriate backend accessor depending on the bootdev
 * driver type:
 *   - mtd_bootdev  -> whole-flash MTD device
 *   - ubi_bootdev  -> MTD partition hosting UBI
 *
 * @dev: bootdev udevice
 *
 * Return: pointer to the relevant struct mtd_info, or NULL if not MTD-based
 */
static inline struct mtd_info *bootdev_get_mtd(struct udevice *dev)
{
	const char *name = dev->driver->name;

	if (IS_ENABLED(CONFIG_BOOTDEV_MTD) && !strcmp(name, "mtd_bootdev"))
		return mtd_bootdev_get_mtd(dev);
	if (IS_ENABLED(CONFIG_BOOTDEV_UBI) && !strcmp(name, "ubi_bootdev"))
		return ubi_bootdev_get_ubi_mtd(dev);

	return NULL;
}

#endif /* __BOOTDEV_MTD_H */

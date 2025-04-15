/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2016 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#ifndef __MESON_BOOT_H__
#define __MESON_BOOT_H__

#include <linux/types.h>

/* Boot device */
#define BOOT_DEVICE_RESERVED    0
#define BOOT_DEVICE_EMMC        1
#define BOOT_DEVICE_NAND        2
#define BOOT_DEVICE_SPI         3
#define BOOT_DEVICE_SD          4
#define BOOT_DEVICE_USB         5

int meson_get_boot_device(void);

int meson_get_soc_rev(char *buff, size_t buff_len);

/**
 * meson_get_socinfo - retrieve cpu_id of the Amlogic SoC
 *
 * The value in the following format is read from register:
 *   +-----------+------------+------------+------------+
 *   | family_id | package_id |  chip_rev  | layout_rev |
 *   +-----------+------------+------------+------------+
 *   | 31     24 | 23      16 | 15       8 | 7        0 |
 *   +-----------+------------+------------+------------+
 *
 * Return: 4 bytes value of cpu_id on success or 0 on failure.
 */
u32 meson_get_socinfo(void);

#endif /* __MESON_BOOT_H__ */

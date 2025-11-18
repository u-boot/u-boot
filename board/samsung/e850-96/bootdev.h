/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2025 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#ifndef __E850_96_BOOTDEV_H
#define __E850_96_BOOTDEV_H

#include <stdbool.h>

enum bootdev {
	BOOTDEV_ERROR,
	BOOTDEV_SD,
	BOOTDEV_EMMC,
	BOOTDEV_USB,
	BOOTDEV_UFS,
};

enum bootdev bootdev_get_current(void);
bool bootdev_is_usb(void);

#endif /* __E850_96_BOOTDEV_H */

/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __GENBOOK_CM5_RK3588_H
#define __GENBOOK_CM5_RK3588_H

#define ROCKCHIP_DEVICE_SETTINGS \
		"stdout=serial,vidconsole\0" \
		"stderr=serial,vidconsole\0"

/*
 * As a laptop, there is no sdmmc, and we want to
 * set usb the highest boot priority for third-part
 * os installation.
 */
#define BOOT_TARGETS "usb mmc0"

#include <configs/rk3588_common.h>

#endif /* __GENBOOK_CM5_RK3588_H */

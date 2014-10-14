/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 */

#include <common.h>
#include <cros_ec.h>
#include <dm.h>
#include <errno.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_DM_CROS_EC
struct local_info {
	struct cros_ec_dev *cros_ec_dev;	/* Pointer to cros_ec device */
	int cros_ec_err;			/* Error for cros_ec, 0 if ok */
};

static struct local_info local;
#endif

struct cros_ec_dev *board_get_cros_ec_dev(void)
{
#ifdef CONFIG_DM_CROS_EC
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	if (ret) {
		debug("%s: Error %d\n", __func__, ret);
		return NULL;
	}
	return dev->uclass_priv;
#else
	return local.cros_ec_dev;
#endif
}

static int board_init_cros_ec_devices(const void *blob)
{
#ifndef CONFIG_DM_CROS_EC
	local.cros_ec_err = cros_ec_init(blob, &local.cros_ec_dev);
	if (local.cros_ec_err)
		return -1;  /* Will report in board_late_init() */
#endif

	return 0;
}

int cros_ec_board_init(void)
{
	return board_init_cros_ec_devices(gd->fdt_blob);
}

int cros_ec_get_error(void)
{
#ifdef CONFIG_DM_CROS_EC
	struct udevice *dev;
	int ret;

	ret = uclass_get_device(UCLASS_CROS_EC, 0, &dev);
	if (ret && ret != -ENODEV)
		return ret;

	return 0;
#else
	return local.cros_ec_err;
#endif
}

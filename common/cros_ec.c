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
DECLARE_GLOBAL_DATA_PTR;

struct local_info {
	struct cros_ec_dev *cros_ec_dev;	/* Pointer to cros_ec device */
	int cros_ec_err;			/* Error for cros_ec, 0 if ok */
};

static struct local_info local;

struct cros_ec_dev *board_get_cros_ec_dev(void)
{
	return local.cros_ec_dev;
}

static int board_init_cros_ec_devices(const void *blob)
{
	local.cros_ec_err = cros_ec_init(blob, &local.cros_ec_dev);
	if (local.cros_ec_err)
		return -1;  /* Will report in board_late_init() */

	return 0;
}

int cros_ec_board_init(void)
{
	return board_init_cros_ec_devices(gd->fdt_blob);
}

int cros_ec_get_error(void)
{
	return local.cros_ec_err;
}

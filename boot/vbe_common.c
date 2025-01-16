// SPDX-License-Identifier: GPL-2.0
/*
 * Verified Boot for Embedded (VBE) common functions
 *
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <part.h>
#include <vsprintf.h>
#include "vbe_common.h"

int vbe_get_blk(const char *storage, struct udevice **blkp)
{
	struct blk_desc *desc;
	char devname[16];
	const char *end;
	int devnum;

	/* First figure out the block device */
	log_debug("storage=%s\n", storage);
	devnum = trailing_strtoln_end(storage, NULL, &end);
	if (devnum == -1)
		return log_msg_ret("num", -ENODEV);
	if (end - storage >= sizeof(devname))
		return log_msg_ret("end", -E2BIG);
	strlcpy(devname, storage, end - storage + 1);
	log_debug("dev=%s, %x\n", devname, devnum);

	desc = blk_get_dev(devname, devnum);
	if (!desc)
		return log_msg_ret("get", -ENXIO);
	*blkp = desc->bdev;

	return 0;
}

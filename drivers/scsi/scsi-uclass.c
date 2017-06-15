/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016 Xilinx, Inc
 * Written by Michal Simek
 *
 * Based on ahci-uclass.c
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <scsi.h>

static int scsi_post_probe(struct udevice *dev)
{
	debug("%s: device %p\n", __func__, dev);
	scsi_low_level_init(0, dev);
	return 0;
}

UCLASS_DRIVER(scsi) = {
	.id		= UCLASS_SCSI,
	.name		= "scsi",
	.post_probe	 = scsi_post_probe,
};

/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * This file contains dummy implementations of SCSI functions requried so
 * that CONFIG_SCSI can be enabled for sandbox.
 */

#include <common.h>
#include <scsi.h>

int scsi_bus_reset(struct udevice *dev)
{
	return 0;
}

void scsi_init(void)
{
}

int scsi_exec(struct udevice *dev, struct scsi_cmd *pccb)
{
	return 0;
}

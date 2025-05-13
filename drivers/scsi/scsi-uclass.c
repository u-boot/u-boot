// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 * Copyright (c) 2016 Xilinx, Inc
 * Written by Michal Simek
 *
 * Based on ahci-uclass.c
 */

#define LOG_CATEGORY UCLASS_SCSI

#include <blk.h>
#include <dm.h>
#include <part.h>
#include <scsi.h>

int scsi_exec(struct udevice *dev, struct scsi_cmd *pccb)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->exec)
		return -ENOSYS;

	return ops->exec(dev, pccb);
}

int scsi_get_blk_by_uuid(const char *uuid,
			 struct blk_desc **blk_desc_ptr,
			 struct disk_partition *part_info_ptr)
{
	static int is_scsi_scanned;
	struct blk_desc *blk;
	int i, ret;

	if (!is_scsi_scanned) {
		scsi_scan(false /* no verbose */);
		is_scsi_scanned = 1;
	}

	for (i = 0; i < blk_find_max_devnum(UCLASS_SCSI) + 1; i++) {
		ret = blk_get_desc(UCLASS_SCSI, i, &blk);
		if (ret)
			continue;

		ret = part_get_info_by_uuid(blk, uuid, part_info_ptr);
		if (ret > 0) {
			*blk_desc_ptr = blk;
			return 0;
		}
	}

	return -1;
}

int scsi_bus_reset(struct udevice *dev)
{
	struct scsi_ops *ops = scsi_get_ops(dev);

	if (!ops->bus_reset)
		return -ENOSYS;

	return ops->bus_reset(dev);
}

UCLASS_DRIVER(scsi) = {
	.id		= UCLASS_SCSI,
	.name		= "scsi",
	.per_device_plat_auto	= sizeof(struct scsi_plat),
};

/*
 * Copyright (C) 2017 NXP Semiconductors
 * Copyright (C) 2017 Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <dm.h>
#include <dm/device.h>
#include "nvme.h"

static int nvme_info_init(struct uclass *uc)
{
	struct nvme_info *info = (struct nvme_info *)uc->priv;

	info->ns_num = 0;
	info->ndev_num = 0;
	INIT_LIST_HEAD(&info->dev_list);
	nvme_info = info;

	return 0;
}

static int nvme_uclass_post_probe(struct udevice *udev)
{
	char name[20];
	struct udevice *ns_udev;
	int i, ret;
	struct nvme_dev *ndev = dev_get_priv(udev);

	/* Create a blk device for each namespace */
	for (i = 0; i < ndev->nn; i++) {
		sprintf(name, "blk#%d", nvme_info->ns_num);

		/* The real blksz and size will be set by nvme_blk_probe() */
		ret = blk_create_devicef(udev, "nvme-blk", name, IF_TYPE_NVME,
					 nvme_info->ns_num++, 512, 0, &ns_udev);
		if (ret) {
			nvme_info->ns_num--;

			return ret;
		}
	}

	return 0;
}

UCLASS_DRIVER(nvme) = {
	.name	= "nvme",
	.id	= UCLASS_NVME,
	.init	= nvme_info_init,
	.post_probe = nvme_uclass_post_probe,
	.priv_auto_alloc_size = sizeof(struct nvme_info),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#define LOG_CATEGORY UCLASS_IOMMU

#include <common.h>
#include <dm.h>

#if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA))
int dev_iommu_enable(struct udevice *dev)
{
	struct ofnode_phandle_args args;
	struct udevice *dev_iommu;
	int i, count, ret = 0;

	count = dev_count_phandle_with_args(dev, "iommus",
					    "#iommu-cells", 0);
	for (i = 0; i < count; i++) {
		ret = dev_read_phandle_with_args(dev, "iommus",
						 "#iommu-cells", 0, i, &args);
		if (ret) {
			debug("%s: dev_read_phandle_with_args failed: %d\n",
			      __func__, ret);
			return ret;
		}

		ret = uclass_get_device_by_ofnode(UCLASS_IOMMU, args.node,
						  &dev_iommu);
		if (ret) {
			debug("%s: uclass_get_device_by_ofnode failed: %d\n",
			      __func__, ret);
			return ret;
		}
	}

	return 0;
}
#endif

UCLASS_DRIVER(iommu) = {
	.id		= UCLASS_IOMMU,
	.name		= "iommu",
};

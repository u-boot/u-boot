// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#define LOG_CATEGORY UCLASS_IOMMU

#include <common.h>
#include <dm.h>
#include <iommu.h>
#include <phys2bus.h>
#include <asm/io.h>

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
		dev->iommu = dev_iommu;
	}

	return 0;
}
#endif

dma_addr_t dev_iommu_dma_map(struct udevice *dev, void *addr, size_t size)
{
	const struct iommu_ops *ops;

	if (dev->iommu) {
		ops = device_get_ops(dev->iommu);
		if (ops && ops->map)
			return ops->map(dev->iommu, addr, size);
	}

	return dev_phys_to_bus(dev, virt_to_phys(addr));
}

void dev_iommu_dma_unmap(struct udevice *dev, dma_addr_t addr, size_t size)
{
	const struct iommu_ops *ops;

	if (dev->iommu) {
		ops = device_get_ops(dev->iommu);
		if (ops && ops->unmap)
			ops->unmap(dev->iommu, addr, size);
	}
}

UCLASS_DRIVER(iommu) = {
	.id		= UCLASS_IOMMU,
	.name		= "iommu",
};

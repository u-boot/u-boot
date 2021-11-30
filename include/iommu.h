#ifndef _IOMMU_H
#define _IOMMU_H

struct udevice;

#if (CONFIG_IS_ENABLED(OF_CONTROL) && !CONFIG_IS_ENABLED(OF_PLATDATA)) && \
	CONFIG_IS_ENABLED(IOMMU)
int dev_iommu_enable(struct udevice *dev);
#else
static inline int dev_iommu_enable(struct udevice *dev)
{
	return 0;
}
#endif

#endif

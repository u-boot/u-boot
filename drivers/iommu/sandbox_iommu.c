// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <dm.h>
#include <iommu.h>
#include <asm/io.h>
#include <asm/test.h>
#include <linux/sizes.h>

static dma_addr_t sandbox_iommu_map(struct udevice *dev, void *addr,
				    size_t size)
{
	phys_addr_t paddr, dva;
	phys_size_t psize, off;

	paddr = ALIGN_DOWN(virt_to_phys(addr), SANDBOX_IOMMU_PAGE_SIZE);
	off = virt_to_phys(addr) - paddr;
	psize = ALIGN(size + off, SANDBOX_IOMMU_PAGE_SIZE);
	dva = (phys_addr_t)SANDBOX_IOMMU_DVA_ADDR;

	return dva + off;
}

static void sandbox_iommu_unmap(struct udevice *dev, dma_addr_t addr,
				size_t size)
{
	phys_addr_t dva;
	phys_size_t psize;

	dva = ALIGN_DOWN(addr, SANDBOX_IOMMU_PAGE_SIZE);
	psize = size + (addr - dva);
	psize = ALIGN(psize, SANDBOX_IOMMU_PAGE_SIZE);
}

static struct iommu_ops sandbox_iommu_ops = {
	.map = sandbox_iommu_map,
	.unmap = sandbox_iommu_unmap,
};

static const struct udevice_id sandbox_iommu_ids[] = {
	{ .compatible = "sandbox,iommu" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_iommu) = {
	.name = "sandbox_iommu",
	.id = UCLASS_IOMMU,
	.of_match = sandbox_iommu_ids,
	.ops = &sandbox_iommu_ops,
};

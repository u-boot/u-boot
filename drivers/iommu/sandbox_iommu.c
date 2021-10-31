// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Mark Kettenis <kettenis@openbsd.org>
 */

#include <common.h>
#include <dm.h>

static const struct udevice_id sandbox_iommu_ids[] = {
	{ .compatible = "sandbox,iommu" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sandbox_iommu) = {
	.name = "sandbox_iommu",
	.id = UCLASS_IOMMU,
	.of_match = sandbox_iommu_ids,
};

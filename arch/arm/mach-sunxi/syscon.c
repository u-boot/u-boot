// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2026 Junhui Liu <junhui.liu@pigmoral.tech>
 */

#include <dm.h>
#include <syscon.h>

static const struct udevice_id sunxi_syscon_ids[] = {
	{ .compatible = "allwinner,sun4i-a10-sram-controller" },
	{ .compatible = "allwinner,sun4i-a10-system-control" },
	{ .compatible = "allwinner,sun5i-a13-system-control" },
	{ .compatible = "allwinner,sun8i-a23-system-control" },
	{ .compatible = "allwinner,sun8i-h3-system-control" },
	{ .compatible = "allwinner,sun20i-d1-system-control" },
	{ .compatible = "allwinner,sun50i-a64-sram-controller" },
	{ .compatible = "allwinner,sun50i-a64-system-control" },
	{ .compatible = "allwinner,sun50i-h5-system-control" },
	{ .compatible = "allwinner,sun50i-h616-system-control" },
	{ .compatible = "allwinner,sun55i-a523-system-control" },
	{ /* sentinel */ },
};

U_BOOT_DRIVER(sunxi_syscon) = {
	.name = "sunxi_syscon",
	.id = UCLASS_SYSCON,
	.of_match = sunxi_syscon_ids,
};

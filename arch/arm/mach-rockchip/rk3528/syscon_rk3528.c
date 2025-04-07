// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

#include <dm.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3528_syscon_ids[] = {
	{ .compatible = "rockchip,rk3528-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3528_syscon) = {
	.name = "rockchip_rk3528_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3528_syscon_ids,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind = dm_scan_fdt_dev,
#endif
};

// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2023 Rockchip Electronics Co., Ltd
 */

#include <dm.h>
#include <asm/arch-rockchip/clock.h>

static const struct udevice_id rk3576_syscon_ids[] = {
	{ .compatible = "rockchip,rk3576-sys-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rk3576-pmu1-grf",  .data = ROCKCHIP_SYSCON_PMUGRF },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3576_syscon) = {
	.name = "rockchip_rk3576_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3576_syscon_ids,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind = dm_scan_fdt_dev,
#endif
};

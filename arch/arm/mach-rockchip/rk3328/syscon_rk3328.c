// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 */

#include <asm/arch-rockchip/clock.h>
#include <dm.h>
#include <syscon.h>

static const struct udevice_id rk3328_syscon_ids[] = {
	{ .compatible = "rockchip,rk3328-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3328_grf) = {
	.name = "rockchip_rk3328_grf",
	.id = UCLASS_SYSCON,
	.of_match = rk3328_syscon_ids,
#if CONFIG_IS_ENABLED(OF_REAL)
	.bind = dm_scan_fdt_dev,
#endif
};

/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch/clock.h>

static const struct udevice_id rk3288_syscon_ids[] = {
	{ .compatible = "rockchip,rk3288-noc", .data = ROCKCHIP_SYSCON_NOC },
	{ .compatible = "rockchip,rk3288-grf", .data = ROCKCHIP_SYSCON_GRF },
	{ .compatible = "rockchip,rk3288-sgrf", .data = ROCKCHIP_SYSCON_SGRF },
	{ .compatible = "rockchip,rk3288-pmu", .data = ROCKCHIP_SYSCON_PMU },
	{ }
};

U_BOOT_DRIVER(syscon_rk3288) = {
	.name = "rk3288_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3288_syscon_ids,
};

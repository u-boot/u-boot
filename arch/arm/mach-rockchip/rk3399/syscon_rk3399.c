/*
 * (C) Copyright 2016 Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <syscon.h>
#include <asm/arch/clock.h>

static const struct udevice_id rk3399_syscon_ids[] = {
	{ .compatible = "rockchip,rk3399-grf", .data = ROCKCHIP_SYSCON_GRF },
};

U_BOOT_DRIVER(syscon_rk3399) = {
	.name = "rk3399_syscon",
	.id = UCLASS_SYSCON,
	.of_match = rk3399_syscon_ids,
};

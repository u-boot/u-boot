// SPDX-License-Identifier: GPL-2.0
/*
 * (C) Copyright 2020 Rockchip Electronics Co., Ltd.
 */

#include <dm.h>
#include <asm/arch-rockchip/cru_rk3576.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
				DM_DRIVER_GET(rockchip_rk3576_cru), devp);
}

void *rockchip_get_cru(void)
{
	return (void *)RK3576_CRU_BASE;
}

// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

#include <dm.h>
#include <asm/arch-rockchip/cru_rk3528.h>

int rockchip_get_clk(struct udevice **devp)
{
	return uclass_get_device_by_driver(UCLASS_CLK,
				DM_DRIVER_GET(rockchip_rk3528_cru), devp);
}

void *rockchip_get_cru(void)
{
	return RK3528_CRU_BASE;
}

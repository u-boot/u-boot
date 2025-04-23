// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

#include <dm.h>
#include <ram.h>
#include <asm/arch-rockchip/sdram.h>

#define PMUGRF_BASE			0xff370000
#define OS_REG18_REG			0x248

static int rk3528_dmc_get_info(struct udevice *dev, struct ram_info *info)
{
	info->base = CFG_SYS_SDRAM_BASE;
	info->size = rockchip_sdram_size(PMUGRF_BASE + OS_REG18_REG);

	return 0;
}

static struct ram_ops rk3528_dmc_ops = {
	.get_info = rk3528_dmc_get_info,
};

static const struct udevice_id rk3528_dmc_ids[] = {
	{ .compatible = "rockchip,rk3528-dmc" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk3528_dmc) = {
	.name = "rockchip_rk3528_dmc",
	.id = UCLASS_RAM,
	.of_match = rk3528_dmc_ids,
	.ops = &rk3528_dmc_ops,
};

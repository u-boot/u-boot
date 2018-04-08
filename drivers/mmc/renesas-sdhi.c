/*
 * Copyright (C) 2018 Marek Vasut <marek.vasut@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <clk.h>
#include <fdtdec.h>
#include <mmc.h>
#include <dm.h>
#include <linux/compat.h>
#include <linux/dma-direction.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <power/regulator.h>
#include <asm/unaligned.h>

#include "matsushita-common.h"

static const struct dm_mmc_ops renesas_sdhi_ops = {
	.send_cmd = matsu_sd_send_cmd,
	.set_ios = matsu_sd_set_ios,
	.get_cd = matsu_sd_get_cd,
};

static const struct udevice_id renesas_sdhi_match[] = {
	{ .compatible = "renesas,sdhi-r8a7790", .data = 0 },
	{ .compatible = "renesas,sdhi-r8a7791", .data = 0 },
	{ .compatible = "renesas,sdhi-r8a7792", .data = 0 },
	{ .compatible = "renesas,sdhi-r8a7793", .data = 0 },
	{ .compatible = "renesas,sdhi-r8a7794", .data = 0 },
	{ .compatible = "renesas,sdhi-r8a7795", .data = MATSU_SD_CAP_64BIT },
	{ .compatible = "renesas,sdhi-r8a7796", .data = MATSU_SD_CAP_64BIT },
	{ .compatible = "renesas,sdhi-r8a77965", .data = MATSU_SD_CAP_64BIT },
	{ .compatible = "renesas,sdhi-r8a77970", .data = MATSU_SD_CAP_64BIT },
	{ .compatible = "renesas,sdhi-r8a77995", .data = MATSU_SD_CAP_64BIT },
	{ /* sentinel */ }
};

static int renesas_sdhi_probe(struct udevice *dev)
{
	u32 quirks = dev_get_driver_data(dev);
	struct fdt_resource reg_res;
	DECLARE_GLOBAL_DATA_PTR;
	int ret;

	ret = fdt_get_resource(gd->fdt_blob, dev_of_offset(dev), "reg",
			       0, &reg_res);
	if (ret < 0) {
		dev_err(dev, "\"reg\" resource not found, ret=%i\n", ret);
		return ret;
	}

	if (quirks == 0 && fdt_resource_size(&reg_res) == 0x100)
		quirks = MATSU_SD_CAP_16BIT;

	return matsu_sd_probe(dev, quirks);
}

U_BOOT_DRIVER(renesas_sdhi) = {
	.name = "renesas-sdhi",
	.id = UCLASS_MMC,
	.of_match = renesas_sdhi_match,
	.bind = matsu_sd_bind,
	.probe = renesas_sdhi_probe,
	.priv_auto_alloc_size = sizeof(struct matsu_sd_priv),
	.platdata_auto_alloc_size = sizeof(struct matsu_sd_plat),
	.ops = &renesas_sdhi_ops,
};

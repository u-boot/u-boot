/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
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

static const struct dm_mmc_ops uniphier_sd_ops = {
	.send_cmd = matsu_sd_send_cmd,
	.set_ios = matsu_sd_set_ios,
	.get_cd = matsu_sd_get_cd,
};

static const struct udevice_id uniphier_sd_match[] = {
	{ .compatible = "socionext,uniphier-sdhc", .data = 0 },
	{ /* sentinel */ }
};

static int uniphier_sd_probe(struct udevice *dev)
{
	return matsu_sd_probe(dev, 0);
}

U_BOOT_DRIVER(uniphier_mmc) = {
	.name = "uniphier-mmc",
	.id = UCLASS_MMC,
	.of_match = uniphier_sd_match,
	.bind = matsu_sd_bind,
	.probe = uniphier_sd_probe,
	.priv_auto_alloc_size = sizeof(struct matsu_sd_priv),
	.platdata_auto_alloc_size = sizeof(struct matsu_sd_plat),
	.ops = &uniphier_sd_ops,
};

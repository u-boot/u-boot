/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/ioport.h>

#include "denali.h"

static const struct udevice_id denali_nand_dt_ids[] = {
	{
		.compatible = "altr,socfpga-denali-nand",
	},
	{
		.compatible = "socionext,uniphier-denali-nand-v5a",
	},
	{
		.compatible = "socionext,uniphier-denali-nand-v5b",
	},
	{ /* sentinel */ }
};

static int denali_dt_probe(struct udevice *dev)
{
	struct denali_nand_info *denali = dev_get_priv(dev);
	struct resource res;
	int ret;

	ret = dev_read_resource_byname(dev, "denali_reg", &res);
	if (ret)
		return ret;

	denali->flash_reg = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "nand_data", &res);
	if (ret)
		return ret;

	denali->flash_mem = devm_ioremap(dev, res.start, resource_size(&res));

	return denali_init(denali);
}

U_BOOT_DRIVER(denali_nand_dt) = {
	.name = "denali-nand-dt",
	.id = UCLASS_MISC,
	.of_match = denali_nand_dt_ids,
	.probe = denali_dt_probe,
	.priv_auto_alloc_size = sizeof(struct denali_nand_info),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(denali_nand_dt),
					  &dev);
	if (ret && ret != -ENODEV)
		printf("Failed to initialize Denali NAND controller. (error %d)\n",
		       ret);
}

/*
 * Copyright (C) 2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <clk.h>
#include <dm.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/printk.h>

#include "denali.h"

struct denali_dt_data {
	unsigned int revision;
	unsigned int caps;
	const struct nand_ecc_caps *ecc_caps;
};

NAND_ECC_CAPS_SINGLE(denali_socfpga_ecc_caps, denali_calc_ecc_bytes,
		     512, 8, 15);
static const struct denali_dt_data denali_socfpga_data = {
	.caps = DENALI_CAP_HW_ECC_FIXUP,
	.ecc_caps = &denali_socfpga_ecc_caps,
};

NAND_ECC_CAPS_SINGLE(denali_uniphier_v5a_ecc_caps, denali_calc_ecc_bytes,
		     1024, 8, 16, 24);
static const struct denali_dt_data denali_uniphier_v5a_data = {
	.caps = DENALI_CAP_HW_ECC_FIXUP |
		DENALI_CAP_DMA_64BIT,
	.ecc_caps = &denali_uniphier_v5a_ecc_caps,
};

NAND_ECC_CAPS_SINGLE(denali_uniphier_v5b_ecc_caps, denali_calc_ecc_bytes,
		     1024, 8, 16);
static const struct denali_dt_data denali_uniphier_v5b_data = {
	.revision = 0x0501,
	.caps = DENALI_CAP_HW_ECC_FIXUP |
		DENALI_CAP_DMA_64BIT,
	.ecc_caps = &denali_uniphier_v5b_ecc_caps,
};

static const struct udevice_id denali_nand_dt_ids[] = {
	{
		.compatible = "altr,socfpga-denali-nand",
		.data = (unsigned long)&denali_socfpga_data,
	},
	{
		.compatible = "socionext,uniphier-denali-nand-v5a",
		.data = (unsigned long)&denali_uniphier_v5a_data,
	},
	{
		.compatible = "socionext,uniphier-denali-nand-v5b",
		.data = (unsigned long)&denali_uniphier_v5b_data,
	},
	{ /* sentinel */ }
};

static int denali_dt_probe(struct udevice *dev)
{
	struct denali_nand_info *denali = dev_get_priv(dev);
	const struct denali_dt_data *data;
	struct clk clk;
	struct resource res;
	int ret;

	data = (void *)dev_get_driver_data(dev);
	if (data) {
		denali->revision = data->revision;
		denali->caps = data->caps;
		denali->ecc_caps = data->ecc_caps;
	}

	denali->dev = dev;

	ret = dev_read_resource_byname(dev, "denali_reg", &res);
	if (ret)
		return ret;

	denali->reg = devm_ioremap(dev, res.start, resource_size(&res));

	ret = dev_read_resource_byname(dev, "nand_data", &res);
	if (ret)
		return ret;

	denali->host = devm_ioremap(dev, res.start, resource_size(&res));

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret)
		return ret;

	denali->clk_x_rate = clk_get_rate(&clk);

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
		pr_err("Failed to initialize Denali NAND controller. (error %d)\n",
		       ret);
}

// SPDX-License-Identifier: GPL-2.0
/*
 * cdns3-starfive.c - StarFive specific Glue layer for Cadence USB Controller
 *
 * Copyright (C) 2024 StarFive Technology Co., Ltd.
 *
 * Author:	Minda Chen <minda.chen@starfivetech.com>
 */

#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/usb/otg.h>
#include <malloc.h>
#include <reset.h>
#include <regmap.h>
#include <syscon.h>

#include "core.h"

#define USB_STRAP_HOST			BIT(17)
#define USB_STRAP_DEVICE		BIT(18)
#define USB_STRAP_MASK			GENMASK(18, 16)

#define USB_SUSPENDM_HOST		BIT(19)
#define USB_SUSPENDM_MASK		BIT(19)

#define USB_MISC_CFG_MASK		GENMASK(23, 20)
#define USB_SUSPENDM_BYPS		BIT(20)
#define USB_PLL_EN			BIT(22)
#define USB_REFCLK_MODE			BIT(23)

struct cdns_starfive {
	struct udevice *dev;
	struct regmap *stg_syscon;
	struct reset_ctl_bulk resets;
	struct clk_bulk clks;
	u32 stg_usb_mode;
	enum usb_dr_mode mode;
};

static void cdns_mode_init(struct cdns_starfive *data, enum usb_dr_mode mode)
{
	unsigned int strap, suspendm;

	regmap_update_bits(data->stg_syscon, data->stg_usb_mode,
			   USB_MISC_CFG_MASK,
			   USB_SUSPENDM_BYPS | USB_PLL_EN | USB_REFCLK_MODE);

	switch (mode) {
	case USB_DR_MODE_HOST:
		strap = USB_STRAP_HOST;
		suspendm = USB_SUSPENDM_HOST;
		break;
	case USB_DR_MODE_PERIPHERAL:
		strap = USB_STRAP_DEVICE;
		suspendm = 0;
		break;
	default:
		return;
	}

	regmap_update_bits(data->stg_syscon, data->stg_usb_mode,
			   USB_SUSPENDM_MASK | USB_STRAP_MASK,
			   strap | suspendm);
}

static void cdns_clk_rst_deinit(struct cdns_starfive *data)
{
	reset_assert_bulk(&data->resets);
	clk_disable_bulk(&data->clks);
}

static int cdns_clk_rst_init(struct cdns_starfive *data)
{
	int ret;

	ret = clk_get_bulk(data->dev, &data->clks);
	if (ret)
		return ret;

	ret = reset_get_bulk(data->dev, &data->resets);
	if (ret)
		goto err_clk;

	ret = clk_enable_bulk(&data->clks);
	if (ret) {
		dev_err(data->dev, "clk enable failed: %d\n", ret);
		goto err_en_clk;
	}

	ret = reset_deassert_bulk(&data->resets);
	if (ret) {
		dev_err(data->dev, "reset deassert failed: %d\n", ret);
		goto err_reset;
	}

	return 0;

err_reset:
	clk_disable_bulk(&data->clks);
err_en_clk:
	reset_release_bulk(&data->resets);
err_clk:
	clk_release_bulk(&data->clks);

	return ret;
}

static int cdns_starfive_get_syscon(struct cdns_starfive *data)
{
	struct ofnode_phandle_args phandle;
	int ret;

	ret = dev_read_phandle_with_args(data->dev, "starfive,stg-syscon", NULL, 1, 0,
					 &phandle);
	if (ret < 0) {
		dev_err(data->dev, "Can't get stg cfg phandle: %d\n", ret);
		return ret;
	}

	data->stg_syscon = syscon_node_to_regmap(phandle.node);
	if (IS_ERR(data->stg_syscon)) {
		dev_err(data->dev, "fail to get regmap: %d\n", (int)PTR_ERR(data->stg_syscon));
		return PTR_ERR(data->stg_syscon);
	}

	data->stg_usb_mode = phandle.args[0];

	return 0;
}

static int cdns_starfive_probe(struct udevice *dev)
{
	struct cdns_starfive *data = dev_get_plat(dev);
	enum usb_dr_mode dr_mode;
	int ret;

	data->dev = dev;

	ret = cdns_starfive_get_syscon(data);
	if (ret)
		return ret;

	dr_mode = usb_get_dr_mode(dev_ofnode(dev));

	data->mode = dr_mode;
	ret = cdns_clk_rst_init(data);
	if (ret) {
		dev_err(data->dev, "clk reset failed: %d\n", ret);
		return ret;
	}
	cdns_mode_init(data, dr_mode);

	return 0;
}

static int cdns_starfive_remove(struct udevice *dev)
{
	struct cdns_starfive *data = dev_get_plat(dev);

	cdns_clk_rst_deinit(data);
	return 0;
}

static const struct udevice_id cdns_starfive_of_match[] = {
	{ .compatible = "starfive,jh7110-usb", },
	{},
};

U_BOOT_DRIVER(cdns_starfive) = {
	.name = "cdns-starfive",
	.id = UCLASS_NOP,
	.of_match = cdns_starfive_of_match,
	.bind = cdns3_bind,
	.probe = cdns_starfive_probe,
	.remove = cdns_starfive_remove,
	.plat_auto	= sizeof(struct cdns_starfive),
	.flags = DM_FLAG_OS_PREPARE,
};

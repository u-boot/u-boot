// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 *
 * Altera SoCFPGA EMAC extras
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <clk.h>
#include <phy.h>
#include <regmap.h>
#include <reset.h>
#include <syscon.h>
#include "designware.h"

#include <asm/arch/system_manager.h>

enum dwmac_type {
	DWMAC_SOCFPGA_GEN5 = 0,
	DWMAC_SOCFPGA_ARRIA10,
	DWMAC_SOCFPGA_STRATIX10,
};

struct dwmac_socfpga_platdata {
	struct dw_eth_pdata	dw_eth_pdata;
	enum dwmac_type		type;
	void			*phy_intf;
};

static int dwmac_socfpga_ofdata_to_platdata(struct udevice *dev)
{
	struct dwmac_socfpga_platdata *pdata = dev_get_platdata(dev);
	struct regmap *regmap;
	struct ofnode_phandle_args args;
	void *range;
	int ret;

	ret = dev_read_phandle_with_args(dev, "altr,sysmgr-syscon", NULL,
					 2, 0, &args);
	if (ret) {
		dev_err(dev, "Failed to get syscon: %d\n", ret);
		return ret;
	}

	if (args.args_count != 2) {
		dev_err(dev, "Invalid number of syscon args\n");
		return -EINVAL;
	}

	regmap = syscon_node_to_regmap(args.node);
	if (IS_ERR(regmap)) {
		ret = PTR_ERR(regmap);
		dev_err(dev, "Failed to get regmap: %d\n", ret);
		return ret;
	}

	range = regmap_get_range(regmap, 0);
	if (!range) {
		dev_err(dev, "Failed to get regmap range\n");
		return -ENOMEM;
	}

	pdata->phy_intf = range + args.args[0];

	/*
	 * Sadly, the Altera DT bindings don't have SoC-specific compatibles,
	 * so we have to guesstimate which SoC we are running on from the
	 * DWMAC version. Luckily, Altera at least updated the DWMAC with
	 * each SoC.
	 */
	if (ofnode_device_is_compatible(dev->node, "snps,dwmac-3.70a"))
		pdata->type = DWMAC_SOCFPGA_GEN5;

	if (ofnode_device_is_compatible(dev->node, "snps,dwmac-3.72a"))
		pdata->type = DWMAC_SOCFPGA_ARRIA10;

	if (ofnode_device_is_compatible(dev->node, "snps,dwmac-3.74a"))
		pdata->type = DWMAC_SOCFPGA_STRATIX10;

	return designware_eth_ofdata_to_platdata(dev);
}

static int dwmac_socfpga_probe(struct udevice *dev)
{
	struct dwmac_socfpga_platdata *pdata = dev_get_platdata(dev);
	struct eth_pdata *edata = &pdata->dw_eth_pdata.eth_pdata;
	struct reset_ctl_bulk reset_bulk;
	int ret;
	u8 modereg;

	if (pdata->type == DWMAC_SOCFPGA_ARRIA10) {
		switch (edata->phy_interface) {
		case PHY_INTERFACE_MODE_MII:
		case PHY_INTERFACE_MODE_GMII:
			modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_GMII_MII;
			break;
		case PHY_INTERFACE_MODE_RMII:
			modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RMII;
			break;
		case PHY_INTERFACE_MODE_RGMII:
			modereg = SYSMGR_EMACGRP_CTRL_PHYSEL_ENUM_RGMII;
			break;
		default:
			dev_err(dev, "Unsupported PHY mode\n");
			return -EINVAL;
		}

		ret = reset_get_bulk(dev, &reset_bulk);
		if (ret) {
			dev_err(dev, "Failed to get reset: %d\n", ret);
			return ret;
		}

		reset_assert_bulk(&reset_bulk);

		clrsetbits_le32(pdata->phy_intf,
				SYSMGR_EMACGRP_CTRL_PHYSEL_MASK,
				modereg);

		reset_release_bulk(&reset_bulk);
	}

	return designware_eth_probe(dev);
}

static const struct udevice_id dwmac_socfpga_ids[] = {
	{ .compatible = "altr,socfpga-stmmac" },
	{ }
};

U_BOOT_DRIVER(dwmac_socfpga) = {
	.name		= "dwmac_socfpga",
	.id		= UCLASS_ETH,
	.of_match	= dwmac_socfpga_ids,
	.ofdata_to_platdata = dwmac_socfpga_ofdata_to_platdata,
	.probe		= dwmac_socfpga_probe,
	.ops		= &designware_eth_ops,
	.priv_auto_alloc_size = sizeof(struct dw_eth_dev),
	.platdata_auto_alloc_size = sizeof(struct dwmac_socfpga_platdata),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA,
};

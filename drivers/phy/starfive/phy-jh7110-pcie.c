// SPDX-License-Identifier: GPL-2.0+
/*
 * StarFive JH7110 PCIe 2.0 PHY driver
 *
 * Copyright (C) 2024 StarFive Technology Co., Ltd.
 * Author: Minda Chen <minda.chen@starfivetech.com>
 */
#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <errno.h>
#include <generic-phy.h>
#include <regmap.h>
#include <soc.h>
#include <syscon.h>
#include <linux/bitops.h>
#include <linux/err.h>

#include "phy-jh7110-usb-syscon.h"

#define PCIE_KVCO_LEVEL_OFF			0x28
#define PCIE_USB3_PHY_PLL_CTL_OFF		0x7c
#define PCIE_USB3_PHY_SS_MODE			BIT(4)
#define PCIE_KVCO_TUNE_SIGNAL_OFF		0x80
#define PHY_KVCO_FINE_TUNE_LEVEL		0x91
#define PHY_KVCO_FINE_TUNE_SIGNALS		0xc

#define PCIE_USB3_PHY_MODE			0x1
#define PCIE_BUS_WIDTH				0x2
#define PCIE_USB3_PHY_ENABLE			0x1
#define PCIE_USB3_PHY_SPLIT			0x1

struct jh7110_pcie_phy {
	struct phy *phy;
	struct regmap *stg_syscon;
	struct regmap *sys_syscon;
	void __iomem *regs;
	struct regmap_field *phy_mode;
	struct regmap_field *bus_width;
	struct regmap_field *usb3_phy_en;
	struct regmap_field *usb_split;
	enum phy_mode mode;
};

static int phy_pcie_mode_set(struct jh7110_pcie_phy *data, bool usb_mode)
{
	unsigned int phy_mode, width, usb3_phy, ss_mode, split;

	/* default is PCIe mode */
	if (!data->stg_syscon || !data->sys_syscon) {
		if (usb_mode) {
			dev_err(data->phy->dev, "doesn't support USB3 mode\n");
			return -EINVAL;
		}
		return 0;
	}

	if (usb_mode) {
		phy_mode = PCIE_USB3_PHY_MODE;
		width = 0;
		usb3_phy = PCIE_USB3_PHY_ENABLE;
		ss_mode = PCIE_USB3_PHY_SS_MODE;
		split = 0;
	} else {
		phy_mode = 0;
		width = PCIE_BUS_WIDTH;
		usb3_phy = 0;
		ss_mode = 0;
		split = PCIE_USB3_PHY_SPLIT;
	}

	regmap_field_write(data->phy_mode, phy_mode);
	regmap_field_write(data->bus_width, width);
	regmap_field_write(data->usb3_phy_en, usb3_phy);
	clrsetbits_le32(data->regs + PCIE_USB3_PHY_PLL_CTL_OFF,
			PCIE_USB3_PHY_SS_MODE, ss_mode);
	regmap_field_write(data->usb_split, split);

	return 0;
}

static void phy_kvco_gain_set(struct jh7110_pcie_phy *phy)
{
	/* PCIe Multi-PHY PLL KVCO Gain fine tune settings: */
	writel(PHY_KVCO_FINE_TUNE_LEVEL, phy->regs + PCIE_KVCO_LEVEL_OFF);
	writel(PHY_KVCO_FINE_TUNE_SIGNALS, phy->regs + PCIE_KVCO_TUNE_SIGNAL_OFF);
}

static int jh7110_pcie_phy_set_mode(struct phy *phy,
				    enum phy_mode mode, int submode)
{
	struct udevice *dev = phy->dev;
	struct jh7110_pcie_phy *pcie_phy = dev_get_priv(dev);
	int ret;

	if (mode == pcie_phy->mode)
		return 0;

	switch (mode) {
	case PHY_MODE_USB_HOST:
	case PHY_MODE_USB_DEVICE:
	case PHY_MODE_USB_OTG:
		ret = phy_pcie_mode_set(pcie_phy, 1);
		if (ret)
			return ret;
		break;
	case PHY_MODE_PCIE:
		phy_pcie_mode_set(pcie_phy, 0);
		break;
	default:
		return -EINVAL;
	}

	dev_dbg(phy->dev, "Changing PHY mode to %d\n", mode);
	pcie_phy->mode = mode;

	return 0;
}

static const struct phy_ops jh7110_pcie_phy_ops = {
	.set_mode	= jh7110_pcie_phy_set_mode,
};

static int phy_stg_regfield_init(struct udevice *dev, int mode, int usb3)
{
	struct jh7110_pcie_phy *phy = dev_get_priv(dev);
	struct reg_field phy_mode = REG_FIELD(mode, 20, 21);
	struct reg_field bus_width = REG_FIELD(usb3, 2, 3);
	struct reg_field usb3_phy_en = REG_FIELD(usb3, 4, 4);

	phy->phy_mode = devm_regmap_field_alloc(dev, phy->stg_syscon, phy_mode);
	if (IS_ERR(phy->phy_mode)) {
		dev_err(dev, "PHY mode reg field init failed\n");
		return PTR_ERR(phy->phy_mode);
	}

	phy->bus_width = devm_regmap_field_alloc(dev, phy->stg_syscon, bus_width);
	if (IS_ERR(phy->bus_width)) {
		dev_err(dev, "PHY bus width reg field init failed\n");
		return PTR_ERR(phy->bus_width);
	}

	phy->usb3_phy_en = devm_regmap_field_alloc(dev, phy->stg_syscon, usb3_phy_en);
	if (IS_ERR(phy->usb3_phy_en)) {
		dev_err(dev, "USB3 PHY enable field init failed\n");
		return PTR_ERR(phy->bus_width);
	}

	return 0;
}

static int phy_sys_regfield_init(struct udevice *dev, int split)
{
	struct jh7110_pcie_phy *phy = dev_get_priv(dev);
	struct reg_field usb_split  = REG_FIELD(split, USB_PDRSTN_SPLIT_BIT, USB_PDRSTN_SPLIT_BIT);

	phy->usb_split = devm_regmap_field_alloc(dev, phy->sys_syscon, usb_split);
	if (IS_ERR(phy->usb_split)) {
		dev_err(dev, "USB split field init failed\n");
		return PTR_ERR(phy->usb_split);
	}

	return 0;
}

static int starfive_pcie_phy_get_syscon(struct udevice *dev)
{
	struct jh7110_pcie_phy *phy = dev_get_priv(dev);
	struct ofnode_phandle_args sys_phandle, stg_phandle;
	int ret;

	/* get corresponding syscon phandle */
	ret = dev_read_phandle_with_args(dev, "starfive,sys-syscon", NULL, 1, 0,
					 &sys_phandle);

	if (ret < 0) {
		dev_err(dev, "Can't get sys cfg phandle: %d\n", ret);
		return ret;
	}

	ret = dev_read_phandle_with_args(dev, "starfive,stg-syscon", NULL, 2, 0,
					 &stg_phandle);

	if (ret < 0) {
		dev_err(dev, "Can't get stg cfg phandle: %d\n", ret);
		return ret;
	}

	phy->sys_syscon = syscon_node_to_regmap(sys_phandle.node);
	/* get syscon register offset */
	if (!IS_ERR(phy->sys_syscon)) {
		ret = phy_sys_regfield_init(dev, SYSCON_USB_PDRSTN_REG_OFFSET);
		if (ret)
			return ret;
	} else {
		phy->sys_syscon = NULL;
	}

	phy->stg_syscon = syscon_node_to_regmap(stg_phandle.node);
	if (!IS_ERR(phy->stg_syscon))
		return phy_stg_regfield_init(dev, stg_phandle.args[0],
					     stg_phandle.args[1]);
	else
		phy->stg_syscon = NULL;

	return 0;
}

int jh7110_pcie_phy_probe(struct udevice *dev)
{
	struct jh7110_pcie_phy *phy = dev_get_priv(dev);
	int rc;

	phy->regs = dev_read_addr_ptr(dev);
	if (!phy->regs)
		return -EINVAL;

	rc = starfive_pcie_phy_get_syscon(dev);
	if (rc)
		return rc;

	phy_kvco_gain_set(phy);

	return 0;
}

static const struct udevice_id jh7110_pcie_phy[] = {
	{ .compatible = "starfive,jh7110-pcie-phy"},
	{},
};

U_BOOT_DRIVER(jh7110_pcie_phy) = {
	.name = "jh7110_pcie_phy",
	.id = UCLASS_PHY,
	.of_match = jh7110_pcie_phy,
	.probe = jh7110_pcie_phy_probe,
	.ops = &jh7110_pcie_phy_ops,
	.priv_auto	= sizeof(struct jh7110_pcie_phy),
};

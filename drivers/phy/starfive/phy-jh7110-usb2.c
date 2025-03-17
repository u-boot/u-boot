// SPDX-License-Identifier: GPL-2.0+
/*
 * StarFive JH7110 USB 2.0 PHY driver
 *
 * Copyright (C) 2024 StarFive Technology Co., Ltd.
 * Author: Minda Chen <minda.chen@starfivetech.com>
 */

#include <asm/io.h>
#include <clk.h>
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

#define USB_LS_KEEPALIVE_OFF		0x4
#define USB_LS_KEEPALIVE_ENABLE		BIT(4)
#define USB_PHY_CLK_RATE		125000000

struct jh7110_usb2_phy {
	struct phy *phy;
	struct regmap *sys_syscon;
	void __iomem *regs;
	struct clk *usb_125m_clk;
	struct clk *app_125m;
	struct regmap_field *usb_split;
	enum phy_mode mode;
};

static void usb2_set_ls_keepalive(struct jh7110_usb2_phy *phy, bool set)
{
	/* Host mode enable the LS speed keep-alive signal */
	clrsetbits_le32(phy->regs + USB_LS_KEEPALIVE_OFF,
			USB_LS_KEEPALIVE_ENABLE,
			set ? USB_LS_KEEPALIVE_ENABLE : 0);
}

static int usb2_phy_set_mode(struct phy *phy,
			     enum phy_mode mode, int submode)
{
	struct udevice *dev = phy->dev;
	struct jh7110_usb2_phy *usb2_phy = dev_get_priv(dev);

	if (mode == usb2_phy->mode)
		return 0;

	switch (mode) {
	case PHY_MODE_USB_HOST:
	case PHY_MODE_USB_DEVICE:
	case PHY_MODE_USB_OTG:
		dev_dbg(dev, "Changing PHY to %d\n", mode);
		usb2_phy->mode = mode;
		usb2_set_ls_keepalive(usb2_phy, (mode != PHY_MODE_USB_DEVICE));
		break;
	default:
		return -EINVAL;
	}

	/* set default split usb 2.0 only mode */
	regmap_field_write(usb2_phy->usb_split, true);

	return 0;
}

static int jh7110_usb2_phy_init(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct jh7110_usb2_phy *usb2_phy = dev_get_priv(dev);
	int ret;

	ret = clk_set_rate(usb2_phy->usb_125m_clk, USB_PHY_CLK_RATE);
	if (ret < 0) {
		dev_err(dev, "Failed to set 125m clock\n");
		return ret;
	}

	return clk_prepare_enable(usb2_phy->app_125m);
}

static int jh7110_usb2_phy_exit(struct phy *phy)
{
	struct udevice *dev = phy->dev;
	struct jh7110_usb2_phy *usb2_phy = dev_get_priv(dev);

	clk_disable_unprepare(usb2_phy->app_125m);

	return 0;
}

struct phy_ops jh7110_usb2_phy_ops = {
	.init     = jh7110_usb2_phy_init,
	.exit     = jh7110_usb2_phy_exit,
	.set_mode = usb2_phy_set_mode,
};

int jh7110_usb2_phy_probe(struct udevice *dev)
{
	struct jh7110_usb2_phy *phy = dev_get_priv(dev);
	ofnode node;
	struct reg_field usb_split;
	int ret;

	phy->regs = dev_read_addr_ptr(dev);
	if (!phy->regs)
		return -EINVAL;

	node = ofnode_by_compatible(ofnode_null(), "starfive,jh7110-sys-syscon");
	if (!ofnode_valid(node)) {
		dev_err(dev, "Can't get syscon dev node\n");
		return -ENODEV;
	}

	phy->sys_syscon = syscon_node_to_regmap(node);
	if (IS_ERR(phy->sys_syscon)) {
		dev_err(dev, "Can't get syscon regmap: %d\n", ret);
		return PTR_ERR(phy->sys_syscon);
	}

	usb_split.reg = SYSCON_USB_PDRSTN_REG_OFFSET;
	usb_split.lsb = USB_PDRSTN_SPLIT_BIT;
	usb_split.msb = USB_PDRSTN_SPLIT_BIT;
	phy->usb_split = devm_regmap_field_alloc(dev, phy->sys_syscon, usb_split);
	if (IS_ERR(phy->usb_split)) {
		dev_err(dev, "USB split field init failed\n");
		return PTR_ERR(phy->usb_split);
	}

	phy->usb_125m_clk = devm_clk_get(dev, "125m");
	if (IS_ERR(phy->usb_125m_clk)) {
		dev_err(dev, "Failed to get 125m clock\n");
		return PTR_ERR(phy->usb_125m_clk);
	}

	phy->app_125m = devm_clk_get(dev, "app_125m");
	if (IS_ERR(phy->app_125m)) {
		dev_err(dev, "Failed to get app 125m clock\n");
		return PTR_ERR(phy->app_125m);
	}

	return 0;
}

static const struct udevice_id jh7110_usb2_phy[] = {
	{ .compatible = "starfive,jh7110-usb-phy"},
	{},
};

U_BOOT_DRIVER(jh7110_usb2_phy) = {
	.name = "jh7110_usb2_phy",
	.id = UCLASS_PHY,
	.of_match = jh7110_usb2_phy,
	.probe = jh7110_usb2_phy_probe,
	.ops = &jh7110_usb2_phy_ops,
	.priv_auto	= sizeof(struct jh7110_usb2_phy),
};

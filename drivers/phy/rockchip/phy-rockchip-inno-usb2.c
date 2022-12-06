// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Rockchip USB2.0 PHY with Innosilicon IP block driver
 *
 * Copyright (C) 2016 Fuzhou Rockchip Electronics Co., Ltd
 * Copyright (C) 2020 Amarula Solutions(India)
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <asm/global_data.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <generic-phy.h>
#include <reset.h>
#include <syscon.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <asm/arch-rockchip/clock.h>

DECLARE_GLOBAL_DATA_PTR;

#define usleep_range(a, b) udelay((b))
#define BIT_WRITEABLE_SHIFT	16

enum rockchip_usb2phy_port_id {
	USB2PHY_PORT_OTG,
	USB2PHY_PORT_HOST,
	USB2PHY_NUM_PORTS,
};

struct usb2phy_reg {
	unsigned int	offset;
	unsigned int	bitend;
	unsigned int	bitstart;
	unsigned int	disable;
	unsigned int	enable;
};

struct rockchip_usb2phy_port_cfg {
	struct usb2phy_reg	phy_sus;
	struct usb2phy_reg	bvalid_det_en;
	struct usb2phy_reg	bvalid_det_st;
	struct usb2phy_reg	bvalid_det_clr;
	struct usb2phy_reg	ls_det_en;
	struct usb2phy_reg	ls_det_st;
	struct usb2phy_reg	ls_det_clr;
	struct usb2phy_reg	utmi_avalid;
	struct usb2phy_reg	utmi_bvalid;
	struct usb2phy_reg	utmi_ls;
	struct usb2phy_reg	utmi_hstdet;
};

struct rockchip_usb2phy_cfg {
	unsigned int reg;
	const struct rockchip_usb2phy_port_cfg port_cfgs[USB2PHY_NUM_PORTS];
};

struct rockchip_usb2phy {
	void *reg_base;
	struct clk phyclk;
	const struct rockchip_usb2phy_cfg *phy_cfg;
};

static inline int property_enable(void *reg_base,
				  const struct usb2phy_reg *reg, bool en)
{
	unsigned int val, mask, tmp;

	tmp = en ? reg->enable : reg->disable;
	mask = GENMASK(reg->bitend, reg->bitstart);
	val = (tmp << reg->bitstart) | (mask << BIT_WRITEABLE_SHIFT);

	return writel(val, reg_base + reg->offset);
}

static const
struct rockchip_usb2phy_port_cfg *us2phy_get_port(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_usb2phy *priv = dev_get_priv(parent);
	const struct rockchip_usb2phy_cfg *phy_cfg = priv->phy_cfg;

	return &phy_cfg->port_cfgs[phy->id];
}

static int rockchip_usb2phy_power_on(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_usb2phy *priv = dev_get_priv(parent);
	const struct rockchip_usb2phy_port_cfg *port_cfg = us2phy_get_port(phy);

	property_enable(priv->reg_base, &port_cfg->phy_sus, false);

	/* waiting for the utmi_clk to become stable */
	usleep_range(1500, 2000);

	return 0;
}

static int rockchip_usb2phy_power_off(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_usb2phy *priv = dev_get_priv(parent);
	const struct rockchip_usb2phy_port_cfg *port_cfg = us2phy_get_port(phy);

	property_enable(priv->reg_base, &port_cfg->phy_sus, true);

	return 0;
}

static int rockchip_usb2phy_init(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_usb2phy *priv = dev_get_priv(parent);
	const struct rockchip_usb2phy_port_cfg *port_cfg = us2phy_get_port(phy);
	int ret;

	ret = clk_enable(&priv->phyclk);
	if (ret && ret != -ENOSYS) {
		dev_err(phy->dev, "failed to enable phyclk (ret=%d)\n", ret);
		return ret;
	}

	if (phy->id == USB2PHY_PORT_OTG) {
		property_enable(priv->reg_base, &port_cfg->bvalid_det_clr, true);
		property_enable(priv->reg_base, &port_cfg->bvalid_det_en, true);
	} else if (phy->id == USB2PHY_PORT_HOST) {
		property_enable(priv->reg_base, &port_cfg->bvalid_det_clr, true);
		property_enable(priv->reg_base, &port_cfg->bvalid_det_en, true);
	}

	return 0;
}

static int rockchip_usb2phy_exit(struct phy *phy)
{
	struct udevice *parent = dev_get_parent(phy->dev);
	struct rockchip_usb2phy *priv = dev_get_priv(parent);

	clk_disable(&priv->phyclk);

	return 0;
}

static int rockchip_usb2phy_of_xlate(struct phy *phy,
				     struct ofnode_phandle_args *args)
{
	const char *name = phy->dev->name;

	if (!strcasecmp(name, "host-port"))
		phy->id = USB2PHY_PORT_HOST;
	else if (!strcasecmp(name, "otg-port"))
		phy->id = USB2PHY_PORT_OTG;
	else
		dev_err(phy->dev, "improper %s device\n", name);

	return 0;
}

static struct phy_ops rockchip_usb2phy_ops = {
	.init = rockchip_usb2phy_init,
	.exit = rockchip_usb2phy_exit,
	.power_on = rockchip_usb2phy_power_on,
	.power_off = rockchip_usb2phy_power_off,
	.of_xlate = rockchip_usb2phy_of_xlate,
};

static int rockchip_usb2phy_probe(struct udevice *dev)
{
	struct rockchip_usb2phy *priv = dev_get_priv(dev);
	const struct rockchip_usb2phy_cfg *phy_cfgs;
	unsigned int reg;
	int index, ret;

	priv->reg_base = syscon_get_first_range(ROCKCHIP_SYSCON_GRF);
	if (IS_ERR(priv->reg_base))
		return PTR_ERR(priv->reg_base);

	ret = ofnode_read_u32(dev_ofnode(dev), "reg", &reg);
	if (ret) {
		dev_err(dev, "failed to read reg property (ret = %d)\n", ret);
		return ret;
	}

	phy_cfgs = (const struct rockchip_usb2phy_cfg *)
					dev_get_driver_data(dev);
	if (!phy_cfgs)
		return -EINVAL;

	/* find out a proper config which can be matched with dt. */
	index = 0;
	while (phy_cfgs[index].reg) {
		if (phy_cfgs[index].reg == reg) {
			priv->phy_cfg = &phy_cfgs[index];
			break;
		}

		++index;
	}

	if (!priv->phy_cfg) {
		dev_err(dev, "failed find proper phy-cfg\n");
		return -EINVAL;
	}

	ret = clk_get_by_name(dev, "phyclk", &priv->phyclk);
	if (ret) {
		dev_err(dev, "failed to get the phyclk (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

static int rockchip_usb2phy_bind(struct udevice *dev)
{
	struct udevice *usb2phy_dev;
	ofnode node;
	const char *name;
	int ret = 0;

	dev_for_each_subnode(node, dev) {
		if (!ofnode_valid(node)) {
			dev_info(dev, "subnode %s not found\n", dev->name);
			return -ENXIO;
		}

		name = ofnode_get_name(node);
		dev_dbg(dev, "subnode %s\n", name);

		ret = device_bind_driver_to_node(dev, "rockchip_usb2phy_port",
						 name, node, &usb2phy_dev);
		if (ret) {
			dev_err(dev,
				"'%s' cannot bind 'rockchip_usb2phy_port'\n", name);
			return ret;
		}
	}

	return ret;
}

static const struct rockchip_usb2phy_cfg rk3399_usb2phy_cfgs[] = {
	{
		.reg		= 0xe450,
		.port_cfgs	= {
			[USB2PHY_PORT_OTG] = {
				.phy_sus	= { 0xe454, 1, 0, 2, 1 },
				.bvalid_det_en	= { 0xe3c0, 3, 3, 0, 1 },
				.bvalid_det_st	= { 0xe3e0, 3, 3, 0, 1 },
				.bvalid_det_clr	= { 0xe3d0, 3, 3, 0, 1 },
				.utmi_avalid	= { 0xe2ac, 7, 7, 0, 1 },
				.utmi_bvalid	= { 0xe2ac, 12, 12, 0, 1 },
			},
			[USB2PHY_PORT_HOST] = {
				.phy_sus	= { 0xe458, 1, 0, 0x2, 0x1 },
				.ls_det_en	= { 0xe3c0, 6, 6, 0, 1 },
				.ls_det_st	= { 0xe3e0, 6, 6, 0, 1 },
				.ls_det_clr	= { 0xe3d0, 6, 6, 0, 1 },
				.utmi_ls	= { 0xe2ac, 22, 21, 0, 1 },
				.utmi_hstdet	= { 0xe2ac, 23, 23, 0, 1 }
			}
		},
	},
	{
		.reg		= 0xe460,
		.port_cfgs	= {
			[USB2PHY_PORT_OTG] = {
				.phy_sus        = { 0xe464, 1, 0, 2, 1 },
				.bvalid_det_en  = { 0xe3c0, 8, 8, 0, 1 },
				.bvalid_det_st  = { 0xe3e0, 8, 8, 0, 1 },
				.bvalid_det_clr = { 0xe3d0, 8, 8, 0, 1 },
				.utmi_avalid	= { 0xe2ac, 10, 10, 0, 1 },
				.utmi_bvalid    = { 0xe2ac, 16, 16, 0, 1 },
			},
			[USB2PHY_PORT_HOST] = {
				.phy_sus	= { 0xe468, 1, 0, 0x2, 0x1 },
				.ls_det_en	= { 0xe3c0, 11, 11, 0, 1 },
				.ls_det_st	= { 0xe3e0, 11, 11, 0, 1 },
				.ls_det_clr	= { 0xe3d0, 11, 11, 0, 1 },
				.utmi_ls	= { 0xe2ac, 26, 25, 0, 1 },
				.utmi_hstdet	= { 0xe2ac, 27, 27, 0, 1 }
			}
		},
	},
	{ /* sentinel */ }
};

static const struct udevice_id rockchip_usb2phy_ids[] = {
	{
		.compatible = "rockchip,rk3399-usb2phy",
		.data = (ulong)&rk3399_usb2phy_cfgs,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(rockchip_usb2phy_port) = {
	.name		= "rockchip_usb2phy_port",
	.id		= UCLASS_PHY,
	.ops		= &rockchip_usb2phy_ops,
};

U_BOOT_DRIVER(rockchip_usb2phy) = {
	.name	= "rockchip_usb2phy",
	.id	= UCLASS_PHY,
	.of_match = rockchip_usb2phy_ids,
	.probe = rockchip_usb2phy_probe,
	.bind = rockchip_usb2phy_bind,
	.priv_auto	= sizeof(struct rockchip_usb2phy),
};

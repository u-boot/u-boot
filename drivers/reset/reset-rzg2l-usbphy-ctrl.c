// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Renesas Electronics Corporation
 */

#include <asm/io.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <renesas/rzg2l-usbphy.h>
#include <reset-uclass.h>
#include <reset.h>

#define RESET			0x000

#define RESET_SEL_PLLRESET	BIT(12)
#define RESET_PLLRESET		BIT(8)

#define RESET_SEL_P2RESET	BIT(5)
#define RESET_SEL_P1RESET	BIT(4)
#define RESET_PHYRST_2		BIT(1)
#define RESET_PHYRST_1		BIT(0)

#define PHY_RESET_MASK          (RESET_PHYRST_1 | RESET_PHYRST_2)

#define NUM_PORTS		2

static int rzg2l_usbphy_ctrl_assert(struct reset_ctl *reset_ctl)
{
	struct rzg2l_usbphy_ctrl_priv *priv = dev_get_priv(reset_ctl->dev);
	u32 val;

	val = readl(priv->regs + RESET);
	val |= reset_ctl->id ? RESET_PHYRST_2 : RESET_PHYRST_1;

	/* If both ports are in reset, we can also place the PLL into reset. */
	if ((val & PHY_RESET_MASK) == PHY_RESET_MASK)
		val |= RESET_PLLRESET;

	writel(val, priv->regs + RESET);
	return 0;
}

static int rzg2l_usbphy_ctrl_deassert(struct reset_ctl *reset_ctl)
{
	struct rzg2l_usbphy_ctrl_priv *priv = dev_get_priv(reset_ctl->dev);
	u32 val = reset_ctl->id ? RESET_PHYRST_2 : RESET_PHYRST_1;

	/* If either port is out of reset, the PLL must also be out of reset. */
	val |= RESET_PLLRESET;

	clrbits_le32(priv->regs + RESET, val);
	return 0;
}

static int rzg2l_usbphy_ctrl_of_xlate(struct reset_ctl *reset_ctl,
				      struct ofnode_phandle_args *args)
{
	if (args->args[0] >= NUM_PORTS)
		return -EINVAL;

	reset_ctl->id = args->args[0];
	return 0;
}

struct reset_ops rzg2l_usbphy_ctrl_ops = {
	.rst_assert = rzg2l_usbphy_ctrl_assert,
	.rst_deassert = rzg2l_usbphy_ctrl_deassert,
	.of_xlate = rzg2l_usbphy_ctrl_of_xlate,
};

static int rzg2l_usbphy_ctrl_probe(struct udevice *dev)
{
	struct rzg2l_usbphy_ctrl_priv *priv = dev_get_priv(dev);
	struct reset_ctl rst;
	int ret;

	priv->regs = dev_read_addr(dev);

	ret = reset_get_by_index(dev, 0, &rst);
	if (ret < 0) {
		dev_err(dev, "failed to get reset line: %d\n", ret);
		return ret;
	}

	ret = reset_deassert(&rst);
	if (ret < 0) {
		dev_err(dev, "failed to de-assert reset line: %d\n", ret);
		return ret;
	}

	/* put pll and phy into reset state */
	setbits_le32(priv->regs + RESET,
		     RESET_SEL_PLLRESET | RESET_PLLRESET |
		     RESET_SEL_P1RESET | RESET_PHYRST_1 |
		     RESET_SEL_P2RESET | RESET_PHYRST_2);

	return 0;
}

static const struct udevice_id rzg2l_usbphy_ctrl_ids[] = {
	{ .compatible = "renesas,rzg2l-usbphy-ctrl", },
	{ /* sentinel */ }
};

static int rzg2l_usbphy_ctrl_bind(struct udevice *dev)
{
	struct driver *drv;
	ofnode node;
	int ret;

	node = ofnode_find_subnode(dev_ofnode(dev), "regulator-vbus");
	if (!ofnode_valid(node)) {
		dev_err(dev, "Failed to find vbus regulator devicetree node\n");
		return -ENOENT;
	}

	drv = lists_driver_lookup_name("rzg2l_usbphy_regulator");
	if (!drv) {
		dev_err(dev, "Failed to find vbus regulator driver\n");
		return -ENOENT;
	}

	ret = device_bind(dev, drv, dev->name, NULL, node, NULL);
	if (ret) {
		dev_err(dev, "Failed to bind vbus regulator: %d\n", ret);
		return ret;
	}

	return 0;
}

U_BOOT_DRIVER(rzg2l_usbphy_ctrl) = {
	.name           = "rzg2l_usbphy_ctrl",
	.id             = UCLASS_RESET,
	.of_match       = rzg2l_usbphy_ctrl_ids,
	.bind           = rzg2l_usbphy_ctrl_bind,
	.probe          = rzg2l_usbphy_ctrl_probe,
	.ops            = &rzg2l_usbphy_ctrl_ops,
	.priv_auto      = sizeof(struct rzg2l_usbphy_ctrl_priv),
};

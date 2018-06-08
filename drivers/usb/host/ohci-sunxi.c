// SPDX-License-Identifier: GPL-2.0+
/*
 * Sunxi ohci glue
 *
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <dm.h>
#include <usb.h>
#include "ohci.h"
#include <generic-phy.h>

#ifdef CONFIG_SUNXI_GEN_SUN4I
#define AHB_CLK_DIST		2
#else
#define AHB_CLK_DIST		1
#endif

struct ohci_sunxi_cfg {
	bool has_reset;
	u32 extra_ahb_gate_mask;
	u32 extra_usb_gate_mask;
};

struct ohci_sunxi_priv {
	struct sunxi_ccm_reg *ccm;
	ohci_t ohci;
	int ahb_gate_mask; /* Mask of ahb_gate0 clk gate bits for this hcd */
	int usb_gate_mask; /* Mask of usb_clk_cfg clk gate bits for this hcd */
	struct phy phy;
	const struct ohci_sunxi_cfg *cfg;
};

static int ohci_usb_probe(struct udevice *dev)
{
	struct usb_bus_priv *bus_priv = dev_get_uclass_priv(dev);
	struct ohci_sunxi_priv *priv = dev_get_priv(dev);
	struct ohci_regs *regs = (struct ohci_regs *)devfdt_get_addr(dev);
	int extra_ahb_gate_mask = 0;
	int phys, ret;

	priv->cfg = (const struct ohci_sunxi_cfg *)dev_get_driver_data(dev);
	priv->ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	if (IS_ERR(priv->ccm))
		return PTR_ERR(priv->ccm);

	phys = dev_count_phandle_with_args(dev, "phys", "#phy-cells");
	if (phys < 0) {
		phys = 0;
		goto no_phy;
	}

	ret = generic_phy_get_by_name(dev, "usb", &priv->phy);
	if (ret) {
		pr_err("failed to get %s usb PHY\n", dev->name);
		return ret;
	}

	ret = generic_phy_init(&priv->phy);
	if (ret) {
		pr_err("failed to init %s USB PHY\n", dev->name);
		return ret;
	}

	ret = generic_phy_power_on(&priv->phy);
	if (ret) {
		pr_err("failed to power on %s USB PHY\n", dev->name);
		return ret;
	}

no_phy:
	bus_priv->companion = true;

	/*
	 * This should go away once we've moved to the driver model for
	 * clocks resp. phys.
	 */
	priv->ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_OHCI0;
	extra_ahb_gate_mask = priv->cfg->extra_ahb_gate_mask;
	priv->usb_gate_mask = CCM_USB_CTRL_OHCI0_CLK;
	priv->ahb_gate_mask <<= phys * AHB_CLK_DIST;
	extra_ahb_gate_mask <<= phys * AHB_CLK_DIST;
	priv->usb_gate_mask <<= phys;

	setbits_le32(&priv->ccm->ahb_gate0,
		     priv->ahb_gate_mask | extra_ahb_gate_mask);
	setbits_le32(&priv->ccm->usb_clk_cfg,
		     priv->usb_gate_mask | priv->cfg->extra_usb_gate_mask);
	if (priv->cfg->has_reset)
		setbits_le32(&priv->ccm->ahb_reset0_cfg,
			     priv->ahb_gate_mask | extra_ahb_gate_mask);

	return ohci_register(dev, regs);
}

static int ohci_usb_remove(struct udevice *dev)
{
	struct ohci_sunxi_priv *priv = dev_get_priv(dev);
	int ret;

	if (generic_phy_valid(&priv->phy)) {
		ret = generic_phy_exit(&priv->phy);
		if (ret) {
			pr_err("failed to exit %s USB PHY\n", dev->name);
			return ret;
		}
	}

	ret = ohci_deregister(dev);
	if (ret)
		return ret;

	if (priv->cfg->has_reset)
		clrbits_le32(&priv->ccm->ahb_reset0_cfg, priv->ahb_gate_mask);
	clrbits_le32(&priv->ccm->usb_clk_cfg, priv->usb_gate_mask);
	clrbits_le32(&priv->ccm->ahb_gate0, priv->ahb_gate_mask);

	return 0;
}

static const struct ohci_sunxi_cfg sun4i_a10_cfg = {
	.has_reset = false,
};

static const struct ohci_sunxi_cfg sun6i_a31_cfg = {
	.has_reset = true,
};

static const struct ohci_sunxi_cfg sun8i_h3_cfg = {
	.has_reset = true,
	.extra_ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0,
};

static const struct ohci_sunxi_cfg sun50i_a64_cfg = {
	.has_reset = true,
	.extra_ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0,
	.extra_usb_gate_mask = CCM_USB_CTRL_OHCI0_CLK,
};

static const struct udevice_id ohci_usb_ids[] = {
	{
		.compatible = "allwinner,sun4i-a10-ohci",
		.data = (ulong)&sun4i_a10_cfg,
	},
	{
		.compatible = "allwinner,sun5i-a13-ohci",
		.data = (ulong)&sun4i_a10_cfg,
	},
	{
		.compatible = "allwinner,sun6i-a31-ohci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun7i-a20-ohci",
		.data = (ulong)&sun4i_a10_cfg,
	},
	{
		.compatible = "allwinner,sun8i-a23-ohci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun8i-a83t-ohci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun8i-h3-ohci",
		.data = (ulong)&sun8i_h3_cfg,
	},
	{
		.compatible = "allwinner,sun9i-a80-ohci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun50i-a64-ohci",
		.data = (ulong)&sun50i_a64_cfg,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(usb_ohci) = {
	.name	= "ohci_sunxi",
	.id	= UCLASS_USB,
	.of_match = ohci_usb_ids,
	.probe = ohci_usb_probe,
	.remove = ohci_usb_remove,
	.ops	= &ohci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ohci_sunxi_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

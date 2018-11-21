// SPDX-License-Identifier: GPL-2.0+
/*
 * Sunxi ehci glue
 *
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2014 Roman Byshko <rbyshko@gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/io.h>
#include <dm.h>
#include "ehci.h"
#include <generic-phy.h>

#ifdef CONFIG_SUNXI_GEN_SUN4I
#define BASE_DIST		0x8000
#define AHB_CLK_DIST		2
#else
#define BASE_DIST		0x1000
#define AHB_CLK_DIST		1
#endif

#define SUN6I_AHB_RESET0_CFG_OFFSET 0x2c0
#define SUN9I_AHB_RESET0_CFG_OFFSET 0x5a0

struct ehci_sunxi_cfg {
	bool has_reset;
	u32 extra_ahb_gate_mask;
	u32 reset0_cfg_offset;
};

struct ehci_sunxi_priv {
	struct ehci_ctrl ehci;
	struct sunxi_ccm_reg *ccm;
	u32 *reset0_cfg;
	int ahb_gate_mask; /* Mask of ahb_gate0 clk gate bits for this hcd */
	struct phy phy;
	const struct ehci_sunxi_cfg *cfg;
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct usb_platdata *plat = dev_get_platdata(dev);
	struct ehci_sunxi_priv *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr = (struct ehci_hccr *)devfdt_get_addr(dev);
	struct ehci_hcor *hcor;
	int extra_ahb_gate_mask = 0;
	u8 reg_mask = 0;
	int phys, ret;

	priv->cfg = (const struct ehci_sunxi_cfg *)dev_get_driver_data(dev);
	priv->ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	if (IS_ERR(priv->ccm))
		return PTR_ERR(priv->ccm);

	priv->reset0_cfg = (void *)priv->ccm +
				   priv->cfg->reset0_cfg_offset;

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
	/*
	 * This should go away once we've moved to the driver model for
	 * clocks resp. phys.
	 */
	reg_mask = ((uintptr_t)hccr - SUNXI_USB1_BASE) / BASE_DIST;
	priv->ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0;
	extra_ahb_gate_mask = priv->cfg->extra_ahb_gate_mask;
	priv->ahb_gate_mask <<= reg_mask * AHB_CLK_DIST;
	extra_ahb_gate_mask <<= reg_mask * AHB_CLK_DIST;

	setbits_le32(&priv->ccm->ahb_gate0,
		     priv->ahb_gate_mask | extra_ahb_gate_mask);
	if (priv->cfg->has_reset)
		setbits_le32(priv->reset0_cfg,
			     priv->ahb_gate_mask | extra_ahb_gate_mask);

	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
				    HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, NULL, 0, plat->init_type);
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct ehci_sunxi_priv *priv = dev_get_priv(dev);
	int ret;

	if (generic_phy_valid(&priv->phy)) {
		ret = generic_phy_exit(&priv->phy);
		if (ret) {
			pr_err("failed to exit %s USB PHY\n", dev->name);
			return ret;
		}
	}

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	if (priv->cfg->has_reset)
		clrbits_le32(priv->reset0_cfg, priv->ahb_gate_mask);
	clrbits_le32(&priv->ccm->ahb_gate0, priv->ahb_gate_mask);

	return 0;
}

static const struct ehci_sunxi_cfg sun4i_a10_cfg = {
	.has_reset = false,
};

static const struct ehci_sunxi_cfg sun6i_a31_cfg = {
	.has_reset = true,
	.reset0_cfg_offset = SUN6I_AHB_RESET0_CFG_OFFSET,
};

static const struct ehci_sunxi_cfg sun8i_h3_cfg = {
	.has_reset = true,
	.extra_ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_OHCI0,
	.reset0_cfg_offset = SUN6I_AHB_RESET0_CFG_OFFSET,
};

static const struct ehci_sunxi_cfg sun9i_a80_cfg = {
	.has_reset = true,
	.reset0_cfg_offset = SUN9I_AHB_RESET0_CFG_OFFSET,
};

static const struct udevice_id ehci_usb_ids[] = {
	{
		.compatible = "allwinner,sun4i-a10-ehci",
		.data = (ulong)&sun4i_a10_cfg,
	},
	{
		.compatible = "allwinner,sun5i-a13-ehci",
		.data = (ulong)&sun4i_a10_cfg,
	},
	{
		.compatible = "allwinner,sun6i-a31-ehci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun7i-a20-ehci",
		.data = (ulong)&sun4i_a10_cfg,
	},
	{
		.compatible = "allwinner,sun8i-a23-ehci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun8i-a83t-ehci",
		.data = (ulong)&sun6i_a31_cfg,
	},
	{
		.compatible = "allwinner,sun8i-h3-ehci",
		.data = (ulong)&sun8i_h3_cfg,
	},
	{
		.compatible = "allwinner,sun9i-a80-ehci",
		.data = (ulong)&sun9i_a80_cfg,
	},
	{
		.compatible = "allwinner,sun50i-a64-ehci",
		.data = (ulong)&sun8i_h3_cfg,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ehci_sunxi) = {
	.name	= "ehci_sunxi",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.platdata_auto_alloc_size = sizeof(struct usb_platdata),
	.priv_auto_alloc_size = sizeof(struct ehci_sunxi_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

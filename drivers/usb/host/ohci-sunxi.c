/*
 * Sunxi ohci glue
 *
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/usb_phy.h>
#include <asm/io.h>
#include <dm.h>
#include <usb.h>
#include "ohci.h"

#ifdef CONFIG_SUNXI_GEN_SUN4I
#define BASE_DIST		0x8000
#define AHB_CLK_DIST		2
#else
#define BASE_DIST		0x1000
#define AHB_CLK_DIST		1
#endif

struct ohci_sunxi_priv {
	ohci_t ohci;
	int ahb_gate_mask; /* Mask of ahb_gate0 clk gate bits for this hcd */
	int usb_gate_mask; /* Mask of usb_clk_cfg clk gate bits for this hcd */
	int phy_index;     /* Index of the usb-phy attached to this hcd */
};

static int ohci_usb_probe(struct udevice *dev)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct usb_bus_priv *bus_priv = dev_get_uclass_priv(dev);
	struct ohci_sunxi_priv *priv = dev_get_priv(dev);
	struct ohci_regs *regs = (struct ohci_regs *)dev_get_addr(dev);
	int extra_ahb_gate_mask = 0;

	bus_priv->companion = true;

	/*
	 * This should go away once we've moved to the driver model for
	 * clocks resp. phys.
	 */
	priv->ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_OHCI0;
#ifdef CONFIG_MACH_SUN8I_H3
	extra_ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0;
#endif
	priv->usb_gate_mask = CCM_USB_CTRL_OHCI0_CLK;
	priv->phy_index = ((uintptr_t)regs - (SUNXI_USB1_BASE + 0x400)) / BASE_DIST;
	priv->ahb_gate_mask <<= priv->phy_index * AHB_CLK_DIST;
	extra_ahb_gate_mask <<= priv->phy_index * AHB_CLK_DIST;
	priv->usb_gate_mask <<= priv->phy_index;
	priv->phy_index++; /* Non otg phys start at 1 */

	setbits_le32(&ccm->ahb_gate0,
		     priv->ahb_gate_mask | extra_ahb_gate_mask);
	setbits_le32(&ccm->usb_clk_cfg, priv->usb_gate_mask);
#ifdef CONFIG_SUNXI_GEN_SUN6I
	setbits_le32(&ccm->ahb_reset0_cfg,
		     priv->ahb_gate_mask | extra_ahb_gate_mask);
#endif

	sunxi_usb_phy_init(priv->phy_index);
	sunxi_usb_phy_power_on(priv->phy_index);

	return ohci_register(dev, regs);
}

static int ohci_usb_remove(struct udevice *dev)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct ohci_sunxi_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ohci_deregister(dev);
	if (ret)
		return ret;

	sunxi_usb_phy_exit(priv->phy_index);

#ifdef CONFIG_SUNXI_GEN_SUN6I
	clrbits_le32(&ccm->ahb_reset0_cfg, priv->ahb_gate_mask);
#endif
	clrbits_le32(&ccm->usb_clk_cfg, priv->usb_gate_mask);
	clrbits_le32(&ccm->ahb_gate0, priv->ahb_gate_mask);

	return 0;
}

static const struct udevice_id ohci_usb_ids[] = {
	{ .compatible = "allwinner,sun4i-a10-ohci", },
	{ .compatible = "allwinner,sun5i-a13-ohci", },
	{ .compatible = "allwinner,sun6i-a31-ohci", },
	{ .compatible = "allwinner,sun7i-a20-ohci", },
	{ .compatible = "allwinner,sun8i-a23-ohci", },
	{ .compatible = "allwinner,sun8i-a83t-ohci", },
	{ .compatible = "allwinner,sun8i-h3-ohci",  },
	{ .compatible = "allwinner,sun9i-a80-ohci", },
	{ .compatible = "allwinner,sun50i-a64-ohci", },
	{ }
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

/*
 * Sunxi ehci glue
 *
 * Copyright (C) 2015 Hans de Goede <hdegoede@redhat.com>
 * Copyright (C) 2014 Roman Byshko <rbyshko@gmail.com>
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
#include "ehci.h"

#ifdef CONFIG_SUNXI_GEN_SUN4I
#define BASE_DIST		0x8000
#define AHB_CLK_DIST		2
#else
#define BASE_DIST		0x1000
#define AHB_CLK_DIST		1
#endif

struct ehci_sunxi_priv {
	struct ehci_ctrl ehci;
	int ahb_gate_mask; /* Mask of ahb_gate0 clk gate bits for this hcd */
	int phy_index;     /* Index of the usb-phy attached to this hcd */
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct usb_platdata *plat = dev_get_platdata(dev);
	struct ehci_sunxi_priv *priv = dev_get_priv(dev);
	struct ehci_hccr *hccr = (struct ehci_hccr *)dev_get_addr(dev);
	struct ehci_hcor *hcor;
	int extra_ahb_gate_mask = 0;

	/*
	 * This should go away once we've moved to the driver model for
	 * clocks resp. phys.
	 */
	priv->ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_EHCI0;
#if defined(CONFIG_MACH_SUN8I_H3) || defined(CONFIG_MACH_SUN50I)
	extra_ahb_gate_mask = 1 << AHB_GATE_OFFSET_USB_OHCI0;
#endif
	priv->phy_index = ((uintptr_t)hccr - SUNXI_USB1_BASE) / BASE_DIST;
	priv->ahb_gate_mask <<= priv->phy_index * AHB_CLK_DIST;
	extra_ahb_gate_mask <<= priv->phy_index * AHB_CLK_DIST;
	priv->phy_index++; /* Non otg phys start at 1 */

	setbits_le32(&ccm->ahb_gate0,
		     priv->ahb_gate_mask | extra_ahb_gate_mask);
#ifdef CONFIG_SUNXI_GEN_SUN6I
	setbits_le32(&ccm->ahb_reset0_cfg,
		     priv->ahb_gate_mask | extra_ahb_gate_mask);
#endif

	sunxi_usb_phy_init(priv->phy_index);
	sunxi_usb_phy_power_on(priv->phy_index);

	hcor = (struct ehci_hcor *)((uintptr_t)hccr +
				    HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	return ehci_register(dev, hccr, hcor, NULL, 0, plat->init_type);
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	struct ehci_sunxi_priv *priv = dev_get_priv(dev);
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	sunxi_usb_phy_exit(priv->phy_index);

#ifdef CONFIG_SUNXI_GEN_SUN6I
	clrbits_le32(&ccm->ahb_reset0_cfg, priv->ahb_gate_mask);
#endif
	clrbits_le32(&ccm->ahb_gate0, priv->ahb_gate_mask);

	return 0;
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "allwinner,sun4i-a10-ehci", },
	{ .compatible = "allwinner,sun5i-a13-ehci", },
	{ .compatible = "allwinner,sun6i-a31-ehci", },
	{ .compatible = "allwinner,sun7i-a20-ehci", },
	{ .compatible = "allwinner,sun8i-a23-ehci", },
	{ .compatible = "allwinner,sun8i-a83t-ehci", },
	{ .compatible = "allwinner,sun8i-h3-ehci",  },
	{ .compatible = "allwinner,sun9i-a80-ehci", },
	{ .compatible = "allwinner,sun50i-a64-ehci", },
	{ }
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

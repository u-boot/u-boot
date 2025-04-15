// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm EHCI driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Based on Linux driver
 */

#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/lists.h>
#include <errno.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <usb/ulpi.h>
#include <wait_bit.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/compat.h>
#include "ehci.h"

struct msm_ehci_priv {
	struct ehci_ctrl ctrl; /* Needed by EHCI */
	struct usb_ehci *ehci; /* Start of IP core*/
	struct ulpi_viewport ulpi_vp; /* ULPI Viewport */
	struct phy phy;
	struct clk iface_clk;
	struct clk core_clk;
};

static int msm_init_after_reset(struct ehci_ctrl *dev)
{
	struct msm_ehci_priv *p = container_of(dev, struct msm_ehci_priv, ctrl);
	struct usb_ehci *ehci = p->ehci;

	generic_phy_reset(&p->phy);

	/* set mode to host controller */
	writel(CM_HOST, &ehci->usbmode);

	return 0;
}

static const struct ehci_ops msm_ehci_ops = {
	.init_after_reset = msm_init_after_reset
};

static int ehci_usb_probe(struct udevice *dev)
{
	struct msm_ehci_priv *p = dev_get_priv(dev);
	struct usb_ehci *ehci = p->ehci;
	struct usb_plat *plat = dev_get_plat(dev);
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	ret = clk_get_by_name(dev, "core", &p->core_clk);
	if (ret) {
		dev_err(dev, "Failed to get core clock: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_name(dev, "iface", &p->iface_clk);
	if (ret) {
		dev_err(dev, "Failed to get iface clock: %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(&p->core_clk);
	if (ret)
		return ret;

	ret = clk_prepare_enable(&p->iface_clk);
	if (ret)
		goto cleanup_core;

	hccr = (struct ehci_hccr *)((phys_addr_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((phys_addr_t)hccr +
			HC_LENGTH(ehci_readl(&(hccr)->cr_capbase)));

	ret = generic_setup_phy(dev, &p->phy, 0, PHY_MODE_USB_HOST, 0);
	if (ret)
		goto cleanup_iface;

	ret = board_usb_init(0, plat->init_type);
	if (ret < 0)
		goto cleanup_iface;

	return ehci_register(dev, hccr, hcor, &msm_ehci_ops, 0,
			     plat->init_type);

cleanup_iface:
	clk_disable_unprepare(&p->iface_clk);
cleanup_core:
	clk_disable_unprepare(&p->core_clk);
	return ret;
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct msm_ehci_priv *p = dev_get_priv(dev);
	struct usb_ehci *ehci = p->ehci;
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	/* Stop controller. */
	clrbits_le32(&ehci->usbcmd, CMD_RUN);

	clk_disable_unprepare(&p->iface_clk);
	clk_disable_unprepare(&p->core_clk);

	ret = generic_shutdown_phy(&p->phy);
	if (ret)
		return ret;

	ret = board_usb_init(0, USB_INIT_DEVICE); /* Board specific hook */
	if (ret < 0)
		return ret;

	/* Reset controller */
	setbits_le32(&ehci->usbcmd, CMD_RESET);

	/* Wait for reset */
	if (wait_for_bit_le32(&ehci->usbcmd, CMD_RESET, false, 30, false)) {
		printf("Stuck on USB reset.\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int ehci_usb_of_to_plat(struct udevice *dev)
{
	struct msm_ehci_priv *priv = dev_get_priv(dev);

	priv->ulpi_vp.port_num = 0;
	priv->ehci = dev_read_addr_ptr(dev);

	if (priv->ehci == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Warning: this will not work if viewport address is > 64 bit due to
	 * ULPI design.
	 */
	priv->ulpi_vp.viewport_addr = (phys_addr_t)&priv->ehci->ulpi_viewpoint;

	return 0;
}

static int ehci_usb_of_bind(struct udevice *dev)
{
	ofnode ulpi_node = ofnode_first_subnode(dev_ofnode(dev));
	ofnode phy_node;

	if (!ofnode_valid(ulpi_node))
		return 0;

	phy_node = ofnode_first_subnode(ulpi_node);
	if (!ofnode_valid(phy_node)) {
		printf("%s: ulpi subnode with no phy\n", __func__);
		return -ENOENT;
	}

	return device_bind_driver_to_node(dev, "msm8916_usbphy", "msm8916_usbphy",
					  phy_node, NULL);
}

#if defined(CONFIG_CI_UDC)
/* Little quirk that MSM needs with Chipidea controller
 * Must reinit phy after reset
 */
void ci_init_after_reset(struct ehci_ctrl *ctrl)
{
	struct msm_ehci_priv *p = ctrl->priv;

	generic_phy_reset(&p->phy);
}
#endif

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "qcom,ci-hdrc", },
	{ }
};

U_BOOT_DRIVER(usb_ehci) = {
	.name	= "ehci_msm",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.of_to_plat = ehci_usb_of_to_plat,
	.bind = ehci_usb_of_bind,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto	= sizeof(struct msm_ehci_priv),
	.plat_auto	= sizeof(struct usb_plat),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

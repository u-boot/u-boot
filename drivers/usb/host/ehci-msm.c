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
	struct phy phy;
};

struct qcom_ci_hdrc_priv {
	struct clk_bulk clks;
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

	hccr = (struct ehci_hccr *)((phys_addr_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((phys_addr_t)hccr +
			HC_LENGTH(ehci_readl(&(hccr)->cr_capbase)));

	ret = generic_setup_phy(dev, &p->phy, 0, PHY_MODE_USB_HOST, 0);
	if (ret)
		return ret;

	ret = board_usb_init(0, plat->init_type);
	if (ret < 0)
		return ret;

	return ehci_register(dev, hccr, hcor, &msm_ehci_ops, 0,
			     plat->init_type);
}

static int ehci_usb_remove(struct udevice *dev)
{
	struct msm_ehci_priv *p = dev_get_priv(dev);
	int ret;

	ret = ehci_deregister(dev);
	if (ret)
		return ret;

	ret = generic_shutdown_phy(&p->phy);
	if (ret)
		return ret;

	ret = board_usb_init(0, USB_INIT_DEVICE); /* Board specific hook */
	if (ret < 0)
		return ret;

	return 0;
}

static int ehci_usb_of_to_plat(struct udevice *dev)
{
	struct msm_ehci_priv *priv = dev_get_priv(dev);

	priv->ehci = dev_read_addr_ptr(dev);

	if (!priv->ehci)
		return -EINVAL;

	return 0;
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

U_BOOT_DRIVER(usb_ehci) = {
	.name	= "ehci_msm",
	.id	= UCLASS_USB,
	.of_to_plat = ehci_usb_of_to_plat,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto	= sizeof(struct msm_ehci_priv),
	.plat_auto	= sizeof(struct usb_plat),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

static int qcom_ci_hdrc_probe(struct udevice *dev)
{
	struct qcom_ci_hdrc_priv *p = dev_get_priv(dev);
	int ret;

	ret = clk_get_bulk(dev, &p->clks);
	if (ret && (ret != -ENOSYS && ret != -ENOENT)) {
		dev_err(dev, "Failed to get clocks: %d\n", ret);
		return ret;
	}

	return clk_enable_bulk(&p->clks);
}

static int qcom_ci_hdrc_remove(struct udevice *dev)
{
	struct qcom_ci_hdrc_priv *p = dev_get_priv(dev);

	return clk_release_bulk(&p->clks);
}

static int qcom_ci_hdrc_bind(struct udevice *dev)
{
	ofnode ulpi_node = ofnode_first_subnode(dev_ofnode(dev));
	ofnode phy_node;
	int ret;

	ret = device_bind_driver_to_node(dev, "ehci_msm", "ehci_msm",
					 dev_ofnode(dev), NULL);
	if (ret)
		return ret;

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

static const struct udevice_id qcom_ci_hdrc_ids[] = {
	{ .compatible = "qcom,ci-hdrc", },
	{ }
};

U_BOOT_DRIVER(qcom_ci_hdrc) = {
	.name		= "qcom_ci_hdrc",
	.id		= UCLASS_NOP,
	.of_match	= qcom_ci_hdrc_ids,
	.bind		= qcom_ci_hdrc_bind,
	.probe		= qcom_ci_hdrc_probe,
	.remove		= qcom_ci_hdrc_remove,
	.priv_auto	= sizeof(struct qcom_ci_hdrc_priv),
};

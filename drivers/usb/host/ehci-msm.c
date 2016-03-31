/*
 * Qualcomm EHCI driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * Based on Linux driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <libfdt.h>
#include <usb.h>
#include <usb/ehci-ci.h>
#include <usb/ulpi.h>
#include <wait_bit.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <linux/compat.h>
#include "ehci.h"

/* PHY viewport regs */
#define ULPI_MISC_A_READ         0x96
#define ULPI_MISC_A_SET          0x97
#define ULPI_MISC_A_CLEAR        0x98
#define ULPI_MISC_A_VBUSVLDEXTSEL    (1 << 1)
#define ULPI_MISC_A_VBUSVLDEXT       (1 << 0)

#define GEN2_SESS_VLD_CTRL_EN (1 << 7)

#define SESS_VLD_CTRL         (1 << 25)

struct msm_ehci_priv {
	struct ehci_ctrl ctrl; /* Needed by EHCI */
	struct usb_ehci *ehci; /* Start of IP core*/
	struct ulpi_viewport ulpi_vp; /* ULPI Viewport */
};

int __weak board_prepare_usb(enum usb_init_type type)
{
	return 0;
}

static void setup_usb_phy(struct msm_ehci_priv *priv)
{
	/* Select and enable external configuration with USB PHY */
	ulpi_write(&priv->ulpi_vp, (u8 *)ULPI_MISC_A_SET,
		   ULPI_MISC_A_VBUSVLDEXTSEL | ULPI_MISC_A_VBUSVLDEXT);
}

static void reset_usb_phy(struct msm_ehci_priv *priv)
{
	/* Disable VBUS mimicing in the controller. */
	ulpi_write(&priv->ulpi_vp, (u8 *)ULPI_MISC_A_CLEAR,
		   ULPI_MISC_A_VBUSVLDEXTSEL | ULPI_MISC_A_VBUSVLDEXT);
}


static int msm_init_after_reset(struct ehci_ctrl *dev)
{
	struct msm_ehci_priv *p = container_of(dev, struct msm_ehci_priv, ctrl);
	struct usb_ehci *ehci = p->ehci;

	/* select ULPI phy */
	writel(PORT_PTS_ULPI, &ehci->portsc);
	setup_usb_phy(p);

	/* Enable sess_vld */
	setbits_le32(&ehci->genconfig2, GEN2_SESS_VLD_CTRL_EN);

	/* Enable external vbus configuration in the LINK */
	setbits_le32(&ehci->usbcmd, SESS_VLD_CTRL);

	/* USB_OTG_HS_AHB_BURST */
	writel(0x0, &ehci->sbuscfg);

	/* USB_OTG_HS_AHB_MODE: HPROT_MODE */
	/* Bus access related config. */
	writel(0x08, &ehci->sbusmode);

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
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	int ret;

	hccr = (struct ehci_hccr *)((phys_addr_t)&ehci->caplength);
	hcor = (struct ehci_hcor *)((phys_addr_t)hccr +
			HC_LENGTH(ehci_readl(&(hccr)->cr_capbase)));

	ret = board_prepare_usb(USB_INIT_HOST);
	if (ret < 0)
		return ret;

	return ehci_register(dev, hccr, hcor, &msm_ehci_ops, 0, USB_INIT_HOST);
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

	reset_usb_phy(p);

	ret = board_prepare_usb(USB_INIT_DEVICE); /* Board specific hook */
	if (ret < 0)
		return ret;

	/* Reset controller */
	setbits_le32(&ehci->usbcmd, CMD_RESET);

	/* Wait for reset */
	if (wait_for_bit(__func__, &ehci->usbcmd, CMD_RESET, false, 30,
			 false)) {
		printf("Stuck on USB reset.\n");
		return -ETIMEDOUT;
	}

	return 0;
}

static int ehci_usb_ofdata_to_platdata(struct udevice *dev)
{
	struct msm_ehci_priv *priv = dev_get_priv(dev);

	priv->ulpi_vp.port_num = 0;
	priv->ehci = (void *)dev_get_addr(dev);

	if (priv->ehci == (void *)FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Warning: this will not work if viewport address is > 64 bit due to
	 * ULPI design.
	 */
	priv->ulpi_vp.viewport_addr = (phys_addr_t)&priv->ehci->ulpi_viewpoint;

	return 0;
}

static const struct udevice_id ehci_usb_ids[] = {
	{ .compatible = "qcom,ehci-host", },
	{ }
};

U_BOOT_DRIVER(usb_ehci) = {
	.name	= "ehci_msm",
	.id	= UCLASS_USB,
	.of_match = ehci_usb_ids,
	.ofdata_to_platdata = ehci_usb_ofdata_to_platdata,
	.probe = ehci_usb_probe,
	.remove = ehci_usb_remove,
	.ops	= &ehci_usb_ops,
	.priv_auto_alloc_size = sizeof(struct msm_ehci_priv),
	.flags	= DM_FLAG_ALLOC_PRIV_DMA,
};

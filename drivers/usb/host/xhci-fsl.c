/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * FSL USB HOST xHCI Controller
 *
 * Author: Ramneek Mehresh<ramneek.mehresh@freescale.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <asm-generic/errno.h>
#include <linux/compat.h>
#include <linux/usb/xhci-fsl.h>
#include <linux/usb/dwc3.h>
#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

static struct fsl_xhci fsl_xhci;
unsigned long ctr_addr[] = FSL_USB_XHCI_ADDR;

__weak int __board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}

void usb_phy_reset(struct dwc3 *dwc3_reg)
{
	/* Assert USB3 PHY reset */
	setbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Assert USB2 PHY reset */
	setbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);

	mdelay(200);

	/* Clear USB3 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb3pipectl[0], DWC3_GUSB3PIPECTL_PHYSOFTRST);

	/* Clear USB2 PHY reset */
	clrbits_le32(&dwc3_reg->g_usb2phycfg, DWC3_GUSB2PHYCFG_PHYSOFTRST);
}

static int fsl_xhci_core_init(struct fsl_xhci *fsl_xhci)
{
	int ret = 0;

	ret = dwc3_core_init(fsl_xhci->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(fsl_xhci->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	/* Set GFLADJ_30MHZ as 20h as per XHCI spec default value */
	dwc3_set_fladj(fsl_xhci->dwc3_reg, GFLADJ_30MHZ_DEFAULT);

	return ret;
}

static int fsl_xhci_core_exit(struct fsl_xhci *fsl_xhci)
{
	/*
	 * Currently fsl socs do not support PHY shutdown from
	 * sw. But this support may be added in future socs.
	 */
	return 0;
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct fsl_xhci *ctx = &fsl_xhci;
	int ret = 0;

	ctx->hcd = (struct xhci_hccr *)ctr_addr[index];
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);

	ret = board_usb_init(index, USB_INIT_HOST);
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	ret = fsl_xhci_core_init(ctx);
	if (ret < 0) {
		puts("Failed to initialize xhci\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)ctx->hcd;
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("fsl-xhci: init hccr %lx and hcor %lx hc_length %lx\n",
	      (uintptr_t)*hccr, (uintptr_t)*hcor,
	      (uintptr_t)HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	struct fsl_xhci *ctx = &fsl_xhci;

	fsl_xhci_core_exit(ctx);
}

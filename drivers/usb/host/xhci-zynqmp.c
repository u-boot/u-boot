/*
 * Copyright 2015 Xilinx, Inc.
 *
 * Zynq USB HOST xHCI Controller
 *
 * Author: Siva Durga Prasad Paladugu <sivadur@xilinx.com>
 *
 * This file was resused from Freescale USB xHCI
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <asm-generic/errno.h>
#include <asm/arch-zynqmp/hardware.h>
#include <linux/compat.h>
#include <linux/usb/xhci-zynqmp.h>
#include <linux/usb/dwc3.h>
#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

static struct zynqmp_xhci zynqmp_xhci;

unsigned long ctr_addr[] = {ZYNQMP_USB0_XHCI_BASEADDR,
			    ZYNQMP_USB1_XHCI_BASEADDR};

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

static int zynqmp_xhci_core_init(struct zynqmp_xhci *zynqmp_xhci)
{
	int ret = 0;

	ret = dwc3_core_init(zynqmp_xhci->dwc3_reg);
	if (ret) {
		debug("%s:failed to initialize core\n", __func__);
		return ret;
	}

	/* We are hard-coding DWC3 core to Host Mode */
	dwc3_set_mode(zynqmp_xhci->dwc3_reg, DWC3_GCTL_PRTCAP_HOST);

	return ret;
}

static int zynqmp_xhci_core_exit(struct zynqmp_xhci *zynqmp_xhci)
{
	/*
	 * Currently zynqmp socs do not support PHY shutdown from
	 * sw. But this support may be added in future socs.
	 */
	return 0;
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct zynqmp_xhci *ctx = &zynqmp_xhci;
	int ret = 0;

	ctx->hcd = (struct xhci_hccr *)ctr_addr[index];
	ctx->dwc3_reg = (struct dwc3 *)((char *)(ctx->hcd) + DWC3_REG_OFFSET);

	ret = board_usb_init(index, USB_INIT_HOST);
	if (ret != 0) {
		puts("Failed to initialize board for USB\n");
		return ret;
	}

	ret = zynqmp_xhci_core_init(ctx);
	if (ret < 0) {
		puts("Failed to initialize xhci\n");
		return ret;
	}

	*hccr = (struct xhci_hccr *)ctx->hcd;
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr
				+ HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	debug("zynqmp-xhci: init hccr %p and hcor %p hc_length %d\n",
	      *hccr, *hcor,
	      HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase)));

	return ret;
}

void xhci_hcd_stop(int index)
{
	struct zynqmp_xhci *ctx = &zynqmp_xhci;

	zynqmp_xhci_core_exit(ctx);
}

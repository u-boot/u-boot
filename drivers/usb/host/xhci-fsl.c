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
#include <fsl_errata.h>
#include <fsl_usb.h>

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

static struct fsl_xhci fsl_xhci;
unsigned long ctr_addr[] = FSL_USB_XHCI_ADDR;

__weak int __board_usb_init(int index, enum usb_init_type init)
{
	return 0;
}

static int erratum_a008751(void)
{
#if defined(CONFIG_TARGET_LS2080AQDS) || defined(CONFIG_TARGET_LS2080ARDB)
	u32 __iomem *scfg = (u32 __iomem *)SCFG_BASE;
	writel(SCFG_USB3PRM1CR_INIT, scfg + SCFG_USB3PRM1CR / 4);
	return 0;
#endif
	return 1;
}

static void fsl_apply_xhci_errata(void)
{
	int ret;
	if (has_erratum_a008751()) {
		ret = erratum_a008751();
		if (ret != 0)
			puts("Failed to apply erratum a008751\n");
	}
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

	fsl_apply_xhci_errata();

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

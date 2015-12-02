/*
 * Copyright 2015 Xilinx, Inc.
 *
 * Zynq USB HOST xHCI Controller
 *
 * Author: Siva Durga Prasad Paladugu<sivadur@xilinx.com>
 *
 * This file was reused from Freescale USB xHCI
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <asm-generic/errno.h>
#include <asm/arch-zynqmp/hardware.h>
#include <linux/compat.h>
#include <linux/usb/dwc3.h>
#include "xhci.h"

/* Declare global data pointer */
DECLARE_GLOBAL_DATA_PTR;

/* Default to the ZYNQMP XHCI defines */
#define USB3_PWRCTL_CLK_CMD_MASK	0x3FE000
#define USB3_PWRCTL_CLK_FREQ_MASK	0xFFC
#define USB3_PHY_PARTIAL_RX_POWERON     BIT(6)
#define USB3_PHY_RX_POWERON		BIT(14)
#define USB3_PHY_TX_POWERON		BIT(15)
#define USB3_PHY_TX_RX_POWERON	(USB3_PHY_RX_POWERON | USB3_PHY_TX_POWERON)
#define USB3_PWRCTL_CLK_CMD_SHIFT   14
#define USB3_PWRCTL_CLK_FREQ_SHIFT	22

/* USBOTGSS_WRAPPER definitions */
#define USBOTGSS_WRAPRESET	BIT(17)
#define USBOTGSS_DMADISABLE BIT(16)
#define USBOTGSS_STANDBYMODE_NO_STANDBY BIT(4)
#define USBOTGSS_STANDBYMODE_SMRT		BIT(5)
#define USBOTGSS_STANDBYMODE_SMRT_WKUP (0x3 << 4)
#define USBOTGSS_IDLEMODE_NOIDLE BIT(2)
#define USBOTGSS_IDLEMODE_SMRT BIT(3)
#define USBOTGSS_IDLEMODE_SMRT_WKUP (0x3 << 2)

/* USBOTGSS_IRQENABLE_SET_0 bit */
#define USBOTGSS_COREIRQ_EN	BIT(1)

/* USBOTGSS_IRQENABLE_SET_1 bits */
#define USBOTGSS_IRQ_SET_1_IDPULLUP_FALL_EN	BIT(1)
#define USBOTGSS_IRQ_SET_1_DISCHRGVBUS_FALL_EN	BIT(3)
#define USBOTGSS_IRQ_SET_1_CHRGVBUS_FALL_EN	BIT(4)
#define USBOTGSS_IRQ_SET_1_DRVVBUS_FALL_EN	BIT(5)
#define USBOTGSS_IRQ_SET_1_IDPULLUP_RISE_EN	BIT(8)
#define USBOTGSS_IRQ_SET_1_DISCHRGVBUS_RISE_EN	BIT(11)
#define USBOTGSS_IRQ_SET_1_CHRGVBUS_RISE_EN	BIT(12)
#define USBOTGSS_IRQ_SET_1_DRVVBUS_RISE_EN	BIT(13)
#define USBOTGSS_IRQ_SET_1_OEVT_EN		BIT(16)
#define USBOTGSS_IRQ_SET_1_DMADISABLECLR_EN	BIT(17)

struct zynqmp_xhci {
	struct xhci_hccr *hcd;
	struct dwc3 *dwc3_reg;
};

static struct zynqmp_xhci zynqmp_xhci;

unsigned long ctr_addr[] = CONFIG_ZYNQMP_XHCI_LIST;

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

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	struct zynqmp_xhci *ctx = &zynqmp_xhci;
	int ret = 0;
	uint32_t hclen;

	if (index < 0 || index >= ARRAY_SIZE(ctr_addr))
		return -EINVAL;

	ctx->hcd = (struct xhci_hccr *)ctr_addr[index];
	ctx->dwc3_reg = (struct dwc3 *)((void *)ctx->hcd + DWC3_REG_OFFSET);

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
	hclen = HC_LENGTH(xhci_readl(&(*hccr)->cr_capbase));
	*hcor = (struct xhci_hcor *)((uintptr_t) *hccr + hclen);

	debug("zynqmp-xhci: init hccr %p and hcor %p hc_length %d\n",
	      *hccr, *hcor, hclen);

	return ret;
}

void xhci_hcd_stop(int index)
{
	/*
	 * Currently zynqmp socs do not support PHY shutdown from
	 * sw. But this support may be added in future socs.
	 */

	return;
}

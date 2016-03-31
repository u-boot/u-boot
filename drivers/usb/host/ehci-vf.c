/*
 * Copyright (c) 2015 Sanchayan Maity <sanchayan.maity@toradex.com>
 * Copyright (C) 2015 Toradex AG
 *
 * Based on ehci-mx6 driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <errno.h>
#include <linux/compiler.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/crm_regs.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/regs-usbphy.h>
#include <usb/ehci-ci.h>

#include "ehci.h"

#define USB_NC_REG_OFFSET				0x00000800

#define ANADIG_PLL_CTRL_EN_USB_CLKS		(1 << 6)

#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UCMD_RUN_STOP		(1 << 0) /* controller run/stop */
#define UCMD_RESET			(1 << 1) /* controller reset */

static const unsigned phy_bases[] = {
	USB_PHY0_BASE_ADDR,
	USB_PHY1_BASE_ADDR,
};

static const unsigned nc_reg_bases[] = {
	USBC0_BASE_ADDR,
	USBC1_BASE_ADDR,
};

static void usb_internal_phy_clock_gate(int index)
{
	void __iomem *phy_reg;

	phy_reg = (void __iomem *)phy_bases[index];
	clrbits_le32(phy_reg + USBPHY_CTRL, USBPHY_CTRL_CLKGATE);
}

static void usb_power_config(int index)
{
	struct anadig_reg __iomem *anadig =
		(struct anadig_reg __iomem *)ANADIG_BASE_ADDR;
	void __iomem *pll_ctrl;

	switch (index) {
	case 0:
		pll_ctrl = &anadig->pll3_ctrl;
		clrbits_le32(pll_ctrl, ANADIG_PLL3_CTRL_BYPASS);
		setbits_le32(pll_ctrl, ANADIG_PLL3_CTRL_ENABLE
			 | ANADIG_PLL3_CTRL_POWERDOWN
			 | ANADIG_PLL_CTRL_EN_USB_CLKS);
		break;
	case 1:
		pll_ctrl = &anadig->pll7_ctrl;
		clrbits_le32(pll_ctrl, ANADIG_PLL7_CTRL_BYPASS);
		setbits_le32(pll_ctrl, ANADIG_PLL7_CTRL_ENABLE
			 | ANADIG_PLL7_CTRL_POWERDOWN
			 | ANADIG_PLL_CTRL_EN_USB_CLKS);
		break;
	default:
		return;
	}
}

static void usb_phy_enable(int index, struct usb_ehci *ehci)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	void __iomem *usb_cmd;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	usb_cmd = (void __iomem *)&ehci->usbcmd;

	/* Stop then Reset */
	clrbits_le32(usb_cmd, UCMD_RUN_STOP);
	while (readl(usb_cmd) & UCMD_RUN_STOP)
		;

	setbits_le32(usb_cmd, UCMD_RESET);
	while (readl(usb_cmd) & UCMD_RESET)
		;

	/* Reset USBPHY module */
	setbits_le32(phy_ctrl, USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Remove CLKGATE and SFTRST */
	clrbits_le32(phy_ctrl, USBPHY_CTRL_CLKGATE | USBPHY_CTRL_SFTRST);
	udelay(10);

	/* Power up the PHY */
	writel(0, phy_reg + USBPHY_PWD);

	/* Enable FS/LS device */
	setbits_le32(phy_ctrl, USBPHY_CTRL_ENUTMILEVEL2 |
		 USBPHY_CTRL_ENUTMILEVEL3);
}

static void usb_oc_config(int index)
{
	void __iomem *ctrl;

	ctrl = (void __iomem *)(nc_reg_bases[index] + USB_NC_REG_OFFSET);

	setbits_le32(ctrl, UCTRL_OVER_CUR_POL);
	setbits_le32(ctrl, UCTRL_OVER_CUR_DIS);
}

int __weak board_usb_phy_mode(int port)
{
	return 0;
}

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;
	enum usb_init_type type;

	if (index >= ARRAY_SIZE(nc_reg_bases))
		return -EINVAL;

	ehci = (struct usb_ehci *)nc_reg_bases[index];

	/* Do board specific initialisation */
	board_ehci_hcd_init(index);

	usb_power_config(index);
	usb_oc_config(index);
	usb_internal_phy_clock_gate(index);
	usb_phy_enable(index, ehci);

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	type = board_usb_phy_mode(index);
	if (type != init)
		return -ENODEV;

	if (init == USB_INIT_DEVICE) {
		setbits_le32(&ehci->usbmode, CM_DEVICE);
		writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	} else if (init == USB_INIT_HOST) {
		setbits_le32(&ehci->usbmode, CM_HOST);
		writel((PORT_PTS_UTMI | PORT_PTS_PTW), &ehci->portsc);
		setbits_le32(&ehci->portsc, USB_EN);
	}

	return 0;
}

int ehci_hcd_stop(int index)
{
	return 0;
}

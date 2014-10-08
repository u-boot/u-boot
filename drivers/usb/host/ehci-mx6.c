/*
 * Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <errno.h>
#include <linux/compiler.h>
#include <usb/ehci-fsl.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/imx-common/iomux-v3.h>

#include "ehci.h"

#define USB_OTGREGS_OFFSET	0x000
#define USB_H1REGS_OFFSET	0x200
#define USB_H2REGS_OFFSET	0x400
#define USB_H3REGS_OFFSET	0x600
#define USB_OTHERREGS_OFFSET	0x800

#define USB_H1_CTRL_OFFSET	0x04

#define USBPHY_CTRL				0x00000030
#define USBPHY_CTRL_SET				0x00000034
#define USBPHY_CTRL_CLR				0x00000038
#define USBPHY_CTRL_TOG				0x0000003c

#define USBPHY_PWD				0x00000000
#define USBPHY_CTRL_SFTRST			0x80000000
#define USBPHY_CTRL_CLKGATE			0x40000000
#define USBPHY_CTRL_ENUTMILEVEL3		0x00008000
#define USBPHY_CTRL_ENUTMILEVEL2		0x00004000
#define USBPHY_CTRL_OTG_ID			0x08000000

#define ANADIG_USB2_CHRG_DETECT_EN_B		0x00100000
#define ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B	0x00080000

#define ANADIG_USB2_PLL_480_CTRL_BYPASS		0x00010000
#define ANADIG_USB2_PLL_480_CTRL_ENABLE		0x00002000
#define ANADIG_USB2_PLL_480_CTRL_POWER		0x00001000
#define ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS	0x00000040


#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UCMD_RUN_STOP           (1 << 0) /* controller run/stop */
#define UCMD_RESET		(1 << 1) /* controller reset */

static const unsigned phy_bases[] = {
	USB_PHY0_BASE_ADDR,
	USB_PHY1_BASE_ADDR,
};

static void usb_internal_phy_clock_gate(int index, int on)
{
	void __iomem *phy_reg;

	if (index >= ARRAY_SIZE(phy_bases))
		return;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_reg += on ? USBPHY_CTRL_CLR : USBPHY_CTRL_SET;
	__raw_writel(USBPHY_CTRL_CLKGATE, phy_reg);
}

static void usb_power_config(int index)
{
	struct anatop_regs __iomem *anatop =
		(struct anatop_regs __iomem *)ANATOP_BASE_ADDR;
	void __iomem *chrg_detect;
	void __iomem *pll_480_ctrl_clr;
	void __iomem *pll_480_ctrl_set;

	switch (index) {
	case 0:
		chrg_detect = &anatop->usb1_chrg_detect;
		pll_480_ctrl_clr = &anatop->usb1_pll_480_ctrl_clr;
		pll_480_ctrl_set = &anatop->usb1_pll_480_ctrl_set;
		break;
	case 1:
		chrg_detect = &anatop->usb2_chrg_detect;
		pll_480_ctrl_clr = &anatop->usb2_pll_480_ctrl_clr;
		pll_480_ctrl_set = &anatop->usb2_pll_480_ctrl_set;
		break;
	default:
		return;
	}
	/*
	 * Some phy and power's special controls
	 * 1. The external charger detector needs to be disabled
	 * or the signal at DP will be poor
	 * 2. The PLL's power and output to usb
	 * is totally controlled by IC, so the Software only needs
	 * to enable them at initializtion.
	 */
	__raw_writel(ANADIG_USB2_CHRG_DETECT_EN_B |
		     ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B,
		     chrg_detect);

	__raw_writel(ANADIG_USB2_PLL_480_CTRL_BYPASS,
		     pll_480_ctrl_clr);

	__raw_writel(ANADIG_USB2_PLL_480_CTRL_ENABLE |
		     ANADIG_USB2_PLL_480_CTRL_POWER |
		     ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS,
		     pll_480_ctrl_set);
}

/* Return 0 : host node, <>0 : device mode */
static int usb_phy_enable(int index, struct usb_ehci *ehci)
{
	void __iomem *phy_reg;
	void __iomem *phy_ctrl;
	void __iomem *usb_cmd;
	u32 val;

	if (index >= ARRAY_SIZE(phy_bases))
		return 0;

	phy_reg = (void __iomem *)phy_bases[index];
	phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	usb_cmd = (void __iomem *)&ehci->usbcmd;

	/* Stop then Reset */
	val = __raw_readl(usb_cmd);
	val &= ~UCMD_RUN_STOP;
	__raw_writel(val, usb_cmd);
	while (__raw_readl(usb_cmd) & UCMD_RUN_STOP)
		;

	val = __raw_readl(usb_cmd);
	val |= UCMD_RESET;
	__raw_writel(val, usb_cmd);
	while (__raw_readl(usb_cmd) & UCMD_RESET)
		;

	/* Reset USBPHY module */
	val = __raw_readl(phy_ctrl);
	val |= USBPHY_CTRL_SFTRST;
	__raw_writel(val, phy_ctrl);
	udelay(10);

	/* Remove CLKGATE and SFTRST */
	val = __raw_readl(phy_ctrl);
	val &= ~(USBPHY_CTRL_CLKGATE | USBPHY_CTRL_SFTRST);
	__raw_writel(val, phy_ctrl);
	udelay(10);

	/* Power up the PHY */
	__raw_writel(0, phy_reg + USBPHY_PWD);
	/* enable FS/LS device */
	val = __raw_readl(phy_ctrl);
	val |= (USBPHY_CTRL_ENUTMILEVEL2 | USBPHY_CTRL_ENUTMILEVEL3);
	__raw_writel(val, phy_ctrl);

	return val & USBPHY_CTRL_OTG_ID;
}

/* Base address for this IP block is 0x02184800 */
struct usbnc_regs {
	u32	ctrl[4];	/* otg/host1-3 */
	u32	uh2_hsic_ctrl;
	u32	uh3_hsic_ctrl;
	u32	otg_phy_ctrl_0;
	u32	uh1_phy_ctrl_0;
};

static void usb_oc_config(int index)
{
	struct usbnc_regs *usbnc = (struct usbnc_regs *)(USB_BASE_ADDR +
			USB_OTHERREGS_OFFSET);
	void __iomem *ctrl = (void __iomem *)(&usbnc->ctrl[index]);
	u32 val;

	val = __raw_readl(ctrl);
#if CONFIG_MACH_TYPE == MACH_TYPE_MX6Q_ARM2
	/* mx6qarm2 seems to required a different setting*/
	val &= ~UCTRL_OVER_CUR_POL;
#else
	val |= UCTRL_OVER_CUR_POL;
#endif
	__raw_writel(val, ctrl);

	val = __raw_readl(ctrl);
	val |= UCTRL_OVER_CUR_DIS;
	__raw_writel(val, ctrl);
}

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

int __weak board_ehci_power(int port, int on)
{
	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	enum usb_init_type type;
	struct usb_ehci *ehci = (struct usb_ehci *)(USB_BASE_ADDR +
		(0x200 * index));

	if (index > 3)
		return -EINVAL;
	enable_usboh3_clk(1);
	mdelay(1);

	/* Do board specific initialization */
	board_ehci_hcd_init(index);

	usb_power_config(index);
	usb_oc_config(index);
	usb_internal_phy_clock_gate(index, 1);
	type = usb_phy_enable(index, ehci) ? USB_INIT_DEVICE : USB_INIT_HOST;

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	if ((type == init) || (type == USB_INIT_DEVICE))
		board_ehci_power(index, (type == USB_INIT_DEVICE) ? 0 : 1);
	if (type != init)
		return -ENODEV;
	if (type == USB_INIT_DEVICE)
		return 0;
	setbits_le32(&ehci->usbmode, CM_HOST);
	__raw_writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mdelay(10);

	return 0;
}

int ehci_hcd_stop(int index)
{
	return 0;
}

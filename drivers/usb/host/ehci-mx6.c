/*
 * Copyright (c) 2009 Daniel Mack <daniel@caiaq.de>
 * Copyright (C) 2010 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include <common.h>
#include <usb.h>
#include <errno.h>
#include <linux/compiler.h>
#include <usb/ehci-fsl.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mx6x_pins.h>
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

#define ANADIG_USB2_CHRG_DETECT_EN_B		0x00100000
#define ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B	0x00080000

#define ANADIG_USB2_PLL_480_CTRL_BYPASS		0x00010000
#define ANADIG_USB2_PLL_480_CTRL_ENABLE		0x00002000
#define ANADIG_USB2_PLL_480_CTRL_POWER		0x00001000
#define ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS	0x00000040


#define UCTRL_OVER_CUR_POL	(1 << 8) /* OTG Polarity of Overcurrent */
#define UCTRL_OVER_CUR_DIS	(1 << 7) /* Disable OTG Overcurrent Detection */

/* USBCMD */
#define UH1_USBCMD_OFFSET	0x140
#define UCMD_RUN_STOP           (1 << 0) /* controller run/stop */
#define UCMD_RESET		(1 << 1) /* controller reset */

static void usbh1_internal_phy_clock_gate(int on)
{
	void __iomem *phy_reg = (void __iomem *)USB_PHY1_BASE_ADDR;

	phy_reg += on ? USBPHY_CTRL_CLR : USBPHY_CTRL_SET;
	__raw_writel(USBPHY_CTRL_CLKGATE, phy_reg);
}

static void usbh1_power_config(void)
{
	struct anatop_regs __iomem *anatop =
		(struct anatop_regs __iomem *)ANATOP_BASE_ADDR;
	/*
	 * Some phy and power's special controls for host1
	 * 1. The external charger detector needs to be disabled
	 * or the signal at DP will be poor
	 * 2. The PLL's power and output to usb for host 1
	 * is totally controlled by IC, so the Software only needs
	 * to enable them at initializtion.
	 */
	__raw_writel(ANADIG_USB2_CHRG_DETECT_EN_B |
		     ANADIG_USB2_CHRG_DETECT_CHK_CHRG_B,
		     &anatop->usb2_chrg_detect);

	__raw_writel(ANADIG_USB2_PLL_480_CTRL_BYPASS,
		     &anatop->usb2_pll_480_ctrl_clr);

	__raw_writel(ANADIG_USB2_PLL_480_CTRL_ENABLE |
		     ANADIG_USB2_PLL_480_CTRL_POWER |
		     ANADIG_USB2_PLL_480_CTRL_EN_USB_CLKS,
		     &anatop->usb2_pll_480_ctrl_set);
}

static int usbh1_phy_enable(void)
{
	void __iomem *phy_reg = (void __iomem *)USB_PHY1_BASE_ADDR;
	void __iomem *phy_ctrl = (void __iomem *)(phy_reg + USBPHY_CTRL);
	void __iomem *usb_cmd =	(void __iomem *)(USBOH3_USB_BASE_ADDR +
						 USB_H1REGS_OFFSET +
						 UH1_USBCMD_OFFSET);
	u32 val;

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
	val = __raw_readl(phy_reg + USBPHY_CTRL);
	val |= (USBPHY_CTRL_ENUTMILEVEL2 | USBPHY_CTRL_ENUTMILEVEL3);
	__raw_writel(val, phy_reg + USBPHY_CTRL);

	return 0;
}

static void usbh1_oc_config(void)
{
	void __iomem *usb_base = (void __iomem *)USBOH3_USB_BASE_ADDR;
	void __iomem *usbother_base = usb_base + USB_OTHERREGS_OFFSET;
	u32 val;

	val = __raw_readl(usbother_base + USB_H1_CTRL_OFFSET);
#if CONFIG_MACH_TYPE == MACH_TYPE_MX6Q_ARM2
	/* mx6qarm2 seems to required a different setting*/
	val &= ~UCTRL_OVER_CUR_POL;
#else
	val |= UCTRL_OVER_CUR_POL;
#endif
	__raw_writel(val, usbother_base + USB_H1_CTRL_OFFSET);

	val = __raw_readl(usbother_base + USB_H1_CTRL_OFFSET);
	val |= UCTRL_OVER_CUR_DIS;
	__raw_writel(val, usbother_base + USB_H1_CTRL_OFFSET);
}

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;

	enable_usboh3_clk(1);
	mdelay(1);

	/* Do board specific initialization */
	board_ehci_hcd_init(CONFIG_MXC_USB_PORT);

#if CONFIG_MXC_USB_PORT == 1
	/* USB Host 1 */
	usbh1_power_config();
	usbh1_oc_config();
	usbh1_internal_phy_clock_gate(1);
	usbh1_phy_enable();
#else
#error "MXC USB port not yet supported"
#endif

	ehci = (struct usb_ehci *)(USBOH3_USB_BASE_ADDR +
		(0x200 * CONFIG_MXC_USB_PORT));
	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
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

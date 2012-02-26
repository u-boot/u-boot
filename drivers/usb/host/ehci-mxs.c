/*
 * Freescale i.MX28 USB Host driver
 *
 * Copyright (C) 2011 Marek Vasut <marek.vasut@gmail.com>
 * on behalf of DENX Software Engineering GmbH
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/regs-common.h>
#include <asm/arch/regs-base.h>
#include <asm/arch/regs-clkctrl.h>
#include <asm/arch/regs-usb.h>
#include <asm/arch/regs-usbphy.h>

#include "ehci-core.h"
#include "ehci.h"

#if	(CONFIG_EHCI_MXS_PORT != 0) && (CONFIG_EHCI_MXS_PORT != 1)
#error	"MXS EHCI: Invalid port selected!"
#endif

#ifndef	CONFIG_EHCI_MXS_PORT
#error	"MXS EHCI: Please define correct port using CONFIG_EHCI_MXS_PORT!"
#endif

static struct ehci_mxs {
	struct mx28_usb_regs	*usb_regs;
	struct mx28_usbphy_regs	*phy_regs;
} ehci_mxs;

int mxs_ehci_get_port(struct ehci_mxs *mxs_usb, int port)
{
	uint32_t usb_base, phy_base;
	switch (port) {
	case 0:
		usb_base = MXS_USBCTRL0_BASE;
		phy_base = MXS_USBPHY0_BASE;
		break;
	case 1:
		usb_base = MXS_USBCTRL1_BASE;
		phy_base = MXS_USBPHY1_BASE;
		break;
	default:
		printf("CONFIG_EHCI_MXS_PORT (port = %d)\n", port);
		return -1;
	}

	mxs_usb->usb_regs = (struct mx28_usb_regs *)usb_base;
	mxs_usb->phy_regs = (struct mx28_usbphy_regs *)phy_base;
	return 0;
}

/* This DIGCTL register ungates clock to USB */
#define	HW_DIGCTL_CTRL			0x8001c000
#define	HW_DIGCTL_CTRL_USB0_CLKGATE	(1 << 2)
#define	HW_DIGCTL_CTRL_USB1_CLKGATE	(1 << 16)

int ehci_hcd_init(void)
{

	int ret;
	uint32_t usb_base, cap_base;
	struct mx28_register_32 *digctl_ctrl =
		(struct mx28_register_32 *)HW_DIGCTL_CTRL;
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;

	ret = mxs_ehci_get_port(&ehci_mxs, CONFIG_EHCI_MXS_PORT);
	if (ret)
		return ret;

	/* Reset the PHY block */
	writel(USBPHY_CTRL_SFTRST, &ehci_mxs.phy_regs->hw_usbphy_ctrl_set);
	udelay(10);
	writel(USBPHY_CTRL_SFTRST | USBPHY_CTRL_CLKGATE,
		&ehci_mxs.phy_regs->hw_usbphy_ctrl_clr);

	/* Enable USB clock */
	writel(CLKCTRL_PLL0CTRL0_EN_USB_CLKS | CLKCTRL_PLL0CTRL0_POWER,
			&clkctrl_regs->hw_clkctrl_pll0ctrl0_set);
	writel(CLKCTRL_PLL1CTRL0_EN_USB_CLKS | CLKCTRL_PLL1CTRL0_POWER,
			&clkctrl_regs->hw_clkctrl_pll1ctrl0_set);

	writel(HW_DIGCTL_CTRL_USB0_CLKGATE | HW_DIGCTL_CTRL_USB1_CLKGATE,
		&digctl_ctrl->reg_clr);

	/* Start USB PHY */
	writel(0, &ehci_mxs.phy_regs->hw_usbphy_pwd);

	/* Enable UTMI+ Level 2 and Level 3 compatibility */
	writel(USBPHY_CTRL_ENUTMILEVEL3 | USBPHY_CTRL_ENUTMILEVEL2 | 1,
		&ehci_mxs.phy_regs->hw_usbphy_ctrl_set);

	usb_base = ((uint32_t)ehci_mxs.usb_regs) + 0x100;
	hccr = (struct ehci_hccr *)usb_base;

	cap_base = ehci_readl(&hccr->cr_capbase);
	hcor = (struct ehci_hcor *)(usb_base + HC_LENGTH(cap_base));

	return 0;
}

int ehci_hcd_stop(void)
{
	int ret;
	uint32_t tmp;
	struct mx28_register_32 *digctl_ctrl =
		(struct mx28_register_32 *)HW_DIGCTL_CTRL;
	struct mx28_clkctrl_regs *clkctrl_regs =
		(struct mx28_clkctrl_regs *)MXS_CLKCTRL_BASE;

	ret = mxs_ehci_get_port(&ehci_mxs, CONFIG_EHCI_MXS_PORT);
	if (ret)
		return ret;

	/* Stop the USB port */
	tmp = ehci_readl(&hcor->or_usbcmd);
	tmp &= ~CMD_RUN;
	ehci_writel(tmp, &hcor->or_usbcmd);

	/* Disable the PHY */
	tmp = USBPHY_PWD_RXPWDRX | USBPHY_PWD_RXPWDDIFF |
		USBPHY_PWD_RXPWD1PT1 | USBPHY_PWD_RXPWDENV |
		USBPHY_PWD_TXPWDV2I | USBPHY_PWD_TXPWDIBIAS |
		USBPHY_PWD_TXPWDFS;
	writel(tmp, &ehci_mxs.phy_regs->hw_usbphy_pwd);

	/* Disable USB clock */
	writel(CLKCTRL_PLL0CTRL0_EN_USB_CLKS,
			&clkctrl_regs->hw_clkctrl_pll0ctrl0_clr);
	writel(CLKCTRL_PLL1CTRL0_EN_USB_CLKS,
			&clkctrl_regs->hw_clkctrl_pll1ctrl0_clr);

	/* Gate off the USB clock */
	writel(HW_DIGCTL_CTRL_USB0_CLKGATE | HW_DIGCTL_CTRL_USB1_CLKGATE,
		&digctl_ctrl->reg_set);

	return 0;
}

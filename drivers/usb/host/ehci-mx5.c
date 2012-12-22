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
#include <asm/arch/mx5x_pins.h>
#include <asm/arch/iomux.h>

#include "ehci.h"

#define MX5_USBOTHER_REGS_OFFSET 0x800


#define MXC_OTG_OFFSET			0
#define MXC_H1_OFFSET			0x200
#define MXC_H2_OFFSET			0x400
#define MXC_H3_OFFSET			0x600

#define MXC_USBCTRL_OFFSET		0
#define MXC_USB_PHY_CTR_FUNC_OFFSET	0x8
#define MXC_USB_PHY_CTR_FUNC2_OFFSET	0xc
#define MXC_USB_CTRL_1_OFFSET		0x10
#define MXC_USBH2CTRL_OFFSET		0x14
#define MXC_USBH3CTRL_OFFSET		0x18

/* USB_CTRL */
/* OTG wakeup intr enable */
#define MXC_OTG_UCTRL_OWIE_BIT		(1 << 27)
/* OTG power mask */
#define MXC_OTG_UCTRL_OPM_BIT		(1 << 24)
/* OTG power pin polarity */
#define MXC_OTG_UCTRL_O_PWR_POL_BIT	(1 << 24)
/* Host1 ULPI interrupt enable */
#define MXC_H1_UCTRL_H1UIE_BIT		(1 << 12)
/* HOST1 wakeup intr enable */
#define MXC_H1_UCTRL_H1WIE_BIT		(1 << 11)
/* HOST1 power mask */
#define MXC_H1_UCTRL_H1PM_BIT		(1 << 8)
/* HOST1 power pin polarity */
#define MXC_H1_UCTRL_H1_PWR_POL_BIT	(1 << 8)

/* USB_PHY_CTRL_FUNC */
/* OTG Polarity of Overcurrent */
#define MXC_OTG_PHYCTRL_OC_POL_BIT	(1 << 9)
/* OTG Disable Overcurrent Event */
#define MXC_OTG_PHYCTRL_OC_DIS_BIT	(1 << 8)
/* UH1 Polarity of Overcurrent */
#define MXC_H1_OC_POL_BIT		(1 << 6)
/* UH1 Disable Overcurrent Event */
#define MXC_H1_OC_DIS_BIT		(1 << 5)
/* OTG Power Pin Polarity */
#define MXC_OTG_PHYCTRL_PWR_POL_BIT	(1 << 3)

/* USBH2CTRL */
#define MXC_H2_UCTRL_H2_OC_POL_BIT	(1 << 31)
#define MXC_H2_UCTRL_H2_OC_DIS_BIT	(1 << 30)
#define MXC_H2_UCTRL_H2UIE_BIT		(1 << 8)
#define MXC_H2_UCTRL_H2WIE_BIT		(1 << 7)
#define MXC_H2_UCTRL_H2PM_BIT		(1 << 4)
#define MXC_H2_UCTRL_H2_PWR_POL_BIT	(1 << 4)

/* USBH3CTRL */
#define MXC_H3_UCTRL_H3_OC_POL_BIT	(1 << 31)
#define MXC_H3_UCTRL_H3_OC_DIS_BIT	(1 << 30)
#define MXC_H3_UCTRL_H3UIE_BIT		(1 << 8)
#define MXC_H3_UCTRL_H3WIE_BIT		(1 << 7)
#define MXC_H3_UCTRL_H3_PWR_POL_BIT	(1 << 4)

/* USB_CTRL_1 */
#define MXC_USB_CTRL_UH1_EXT_CLK_EN	(1 << 25)

/* USB pin configuration */
#define USB_PAD_CONFIG	(PAD_CTL_PKE_ENABLE | PAD_CTL_SRE_FAST | \
			PAD_CTL_DRV_HIGH | PAD_CTL_100K_PU | \
			PAD_CTL_HYS_ENABLE | PAD_CTL_PUE_PULL)

#ifdef CONFIG_MX51
/*
 * Configure the MX51 USB H1 IOMUX
 */
void setup_iomux_usb_h1(void)
{
	mxc_request_iomux(MX51_PIN_USBH1_STP, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_STP, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_CLK, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_CLK, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DIR, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DIR, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_NXT, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_NXT, USB_PAD_CONFIG);

	mxc_request_iomux(MX51_PIN_USBH1_DATA0, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA0, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA1, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA1, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA2, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA2, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA3, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA3, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA4, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA4, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA5, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA5, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA6, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA6, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_USBH1_DATA7, IOMUX_CONFIG_ALT0);
	mxc_iomux_set_pad(MX51_PIN_USBH1_DATA7, USB_PAD_CONFIG);
}

/*
 * Configure the MX51 USB H2 IOMUX
 */
void setup_iomux_usb_h2(void)
{
	mxc_request_iomux(MX51_PIN_EIM_A24, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_A24, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_A25, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_A25, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_A26, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_A26, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_A27, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_A27, USB_PAD_CONFIG);

	mxc_request_iomux(MX51_PIN_EIM_D16, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D16, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D17, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D17, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D18, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D18, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D19, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D19, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D20, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D20, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D21, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D21, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D22, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D22, USB_PAD_CONFIG);
	mxc_request_iomux(MX51_PIN_EIM_D23, IOMUX_CONFIG_ALT2);
	mxc_iomux_set_pad(MX51_PIN_EIM_D23, USB_PAD_CONFIG);
}
#endif

int mxc_set_usbcontrol(int port, unsigned int flags)
{
	unsigned int v;
	void __iomem *usb_base = (void __iomem *)OTG_BASE_ADDR;
	void __iomem *usbother_base;
	int ret = 0;

	usbother_base = usb_base + MX5_USBOTHER_REGS_OFFSET;

	switch (port) {
	case 0:	/* OTG port */
		if (flags & MXC_EHCI_INTERNAL_PHY) {
			v = __raw_readl(usbother_base +
					MXC_USB_PHY_CTR_FUNC_OFFSET);
			if (flags & MXC_EHCI_OC_PIN_ACTIVE_LOW)
				v |= MXC_OTG_PHYCTRL_OC_POL_BIT;
			else
				v &= ~MXC_OTG_PHYCTRL_OC_POL_BIT;
			if (flags & MXC_EHCI_POWER_PINS_ENABLED)
				/* OC/USBPWR is used */
				v &= ~MXC_OTG_PHYCTRL_OC_DIS_BIT;
			else
				/* OC/USBPWR is not used */
				v |= MXC_OTG_PHYCTRL_OC_DIS_BIT;
#ifdef CONFIG_MX51
			if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
				v |= MXC_OTG_PHYCTRL_PWR_POL_BIT;
			else
				v &= ~MXC_OTG_PHYCTRL_PWR_POL_BIT;
#endif
			__raw_writel(v, usbother_base +
					MXC_USB_PHY_CTR_FUNC_OFFSET);

			v = __raw_readl(usbother_base + MXC_USBCTRL_OFFSET);
#ifdef CONFIG_MX51
			if (flags & MXC_EHCI_POWER_PINS_ENABLED)
				v &= ~MXC_OTG_UCTRL_OPM_BIT;
			else
				v |= MXC_OTG_UCTRL_OPM_BIT;
#endif
#ifdef CONFIG_MX53
			if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
				v |= MXC_OTG_UCTRL_O_PWR_POL_BIT;
			else
				v &= ~MXC_OTG_UCTRL_O_PWR_POL_BIT;
#endif
			__raw_writel(v, usbother_base + MXC_USBCTRL_OFFSET);
		}
		break;
	case 1:	/* Host 1 ULPI */
#ifdef CONFIG_MX51
		/* The clock for the USBH1 ULPI port will come externally
		   from the PHY. */
		v = __raw_readl(usbother_base + MXC_USB_CTRL_1_OFFSET);
		__raw_writel(v | MXC_USB_CTRL_UH1_EXT_CLK_EN, usbother_base +
				MXC_USB_CTRL_1_OFFSET);
#endif

		v = __raw_readl(usbother_base + MXC_USBCTRL_OFFSET);
#ifdef CONFIG_MX51
		if (flags & MXC_EHCI_POWER_PINS_ENABLED)
			v &= ~MXC_H1_UCTRL_H1PM_BIT; /* H1 power mask unused */
		else
			v |= MXC_H1_UCTRL_H1PM_BIT; /* H1 power mask used */
#endif
#ifdef CONFIG_MX53
		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MXC_H1_UCTRL_H1_PWR_POL_BIT;
		else
			v &= ~MXC_H1_UCTRL_H1_PWR_POL_BIT;
#endif
		__raw_writel(v, usbother_base + MXC_USBCTRL_OFFSET);

		v = __raw_readl(usbother_base + MXC_USB_PHY_CTR_FUNC_OFFSET);
		if (flags & MXC_EHCI_OC_PIN_ACTIVE_LOW)
			v |= MXC_H1_OC_POL_BIT;
		else
			v &= ~MXC_H1_OC_POL_BIT;
		if (flags & MXC_EHCI_POWER_PINS_ENABLED)
			v &= ~MXC_H1_OC_DIS_BIT; /* OC is used */
		else
			v |= MXC_H1_OC_DIS_BIT; /* OC is not used */
		__raw_writel(v, usbother_base + MXC_USB_PHY_CTR_FUNC_OFFSET);

		break;
	case 2: /* Host 2 ULPI */
		v = __raw_readl(usbother_base + MXC_USBH2CTRL_OFFSET);
#ifdef CONFIG_MX51
		if (flags & MXC_EHCI_POWER_PINS_ENABLED)
			v &= ~MXC_H2_UCTRL_H2PM_BIT; /* H2 power mask unused */
		else
			v |= MXC_H2_UCTRL_H2PM_BIT; /* H2 power mask used */
#endif
#ifdef CONFIG_MX53
		if (flags & MXC_EHCI_OC_PIN_ACTIVE_LOW)
			v |= MXC_H2_UCTRL_H2_OC_POL_BIT;
		else
			v &= ~MXC_H2_UCTRL_H2_OC_POL_BIT;
		if (flags & MXC_EHCI_POWER_PINS_ENABLED)
			v &= ~MXC_H2_UCTRL_H2_OC_DIS_BIT; /* OC is used */
		else
			v |= MXC_H2_UCTRL_H2_OC_DIS_BIT; /* OC is not used */
		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MXC_H2_UCTRL_H2_PWR_POL_BIT;
		else
			v &= ~MXC_H2_UCTRL_H2_PWR_POL_BIT;
#endif
		__raw_writel(v, usbother_base + MXC_USBH2CTRL_OFFSET);
		break;
#ifdef CONFIG_MX53
	case 3: /* Host 3 ULPI */
		v = __raw_readl(usbother_base + MXC_USBH3CTRL_OFFSET);
		if (flags & MXC_EHCI_OC_PIN_ACTIVE_LOW)
			v |= MXC_H3_UCTRL_H3_OC_POL_BIT;
		else
			v &= ~MXC_H3_UCTRL_H3_OC_POL_BIT;
		if (flags & MXC_EHCI_POWER_PINS_ENABLED)
			v &= ~MXC_H3_UCTRL_H3_OC_DIS_BIT; /* OC is used */
		else
			v |= MXC_H3_UCTRL_H3_OC_DIS_BIT; /* OC is not used */
		if (flags & MXC_EHCI_PWR_PIN_ACTIVE_HIGH)
			v |= MXC_H3_UCTRL_H3_PWR_POL_BIT;
		else
			v &= ~MXC_H3_UCTRL_H3_PWR_POL_BIT;
		__raw_writel(v, usbother_base + MXC_USBH3CTRL_OFFSET);
		break;
#endif
	}

	return ret;
}

int __weak board_ehci_hcd_init(int port)
{
	return 0;
}

void __weak board_ehci_hcd_postinit(struct usb_ehci *ehci, int port)
{
}

int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;
#ifdef CONFIG_MX53
	struct clkctl *sc_regs = (struct clkctl *)CCM_BASE_ADDR;
	u32 reg;

	reg = __raw_readl(&sc_regs->cscmr1) & ~(1 << 26);
	/* derive USB PHY clock multiplexer from PLL3 */
	reg |= 1 << 26;
	__raw_writel(reg, &sc_regs->cscmr1);
#endif

	set_usboh3_clk();
	enable_usboh3_clk(1);
	set_usb_phy_clk();
	enable_usb_phy1_clk(1);
	enable_usb_phy2_clk(1);
	mdelay(1);

	/* Do board specific initialization */
	board_ehci_hcd_init(CONFIG_MXC_USB_PORT);

	ehci = (struct usb_ehci *)(OTG_BASE_ADDR +
		(0x200 * CONFIG_MXC_USB_PORT));
	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));
	setbits_le32(&ehci->usbmode, CM_HOST);

	__raw_writel(CONFIG_MXC_USB_PORTSC, &ehci->portsc);
	setbits_le32(&ehci->portsc, USB_EN);

	mxc_set_usbcontrol(CONFIG_MXC_USB_PORT, CONFIG_MXC_USB_FLAGS);
	mdelay(10);

	/* Do board specific post-initialization */
	board_ehci_hcd_postinit(ehci, CONFIG_MXC_USB_PORT);

	return 0;
}

int ehci_hcd_stop(int index)
{
	return 0;
}

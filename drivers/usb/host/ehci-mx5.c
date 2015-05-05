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

__weak void mx5_ehci_powerup_fixup(struct ehci_ctrl *ctrl, uint32_t *status_reg,
				   uint32_t *reg)
{
	mdelay(50);
}

static const struct ehci_ops mx5_ehci_ops = {
	.powerup_fixup		= mx5_ehci_powerup_fixup,
};

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;

	/* The only user for this is efikamx-usb */
	ehci_set_controller_priv(index, NULL, &mx5_ehci_ops);
	set_usboh3_clk();
	enable_usboh3_clk(true);
	set_usb_phy_clk();
	enable_usb_phy1_clk(true);
	enable_usb_phy2_clk(true);
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

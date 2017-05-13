/*
 * drivers/usb/host/ehci-rcar_gen3.
 *	This file is EHCI HCD (Host Controller Driver) for USB.
 *
 * Copyright (C) 2015-2017 Renesas Electronics Corporation
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <usb/ehci-ci.h>
#include "ehci.h"

#define RCAR_GEN3_USB_BASE(n)	(0xEE080000 + ((n) * 0x20000))

#define EHCI_USBCMD		0x120

#define CORE_SPD_RSM_TIMSET	0x30c
#define CORE_OC_TIMSET		0x310

/* Register offset */
#define AHB_OFFSET		0x200

#define BASE_HSUSB		0xE6590000
#define REG_LPSTS		(BASE_HSUSB + 0x0102)	/* 16bit */
#define SUSPM			0x4000
#define SUSPM_NORMAL		BIT(14)
#define REG_UGCTRL2		(BASE_HSUSB + 0x0184)	/* 32bit */
#define USB0SEL			0x00000030
#define USB0SEL_EHCI		0x00000010

#define SMSTPCR7		0xE615014C
#define SMSTPCR700		BIT(0)	/* EHCI3 */
#define SMSTPCR701		BIT(1)	/* EHCI2 */
#define SMSTPCR702		BIT(2)	/* EHCI1 */
#define SMSTPCR703		BIT(3)	/* EHCI0 */
#define SMSTPCR704		BIT(4)	/* HSUSB */

#define AHB_PLL_RST		BIT(1)

#define USBH_INTBEN		BIT(2)
#define USBH_INTAEN		BIT(1)

#define AHB_INT_ENABLE		0x200
#define AHB_USBCTR		0x20c

int ehci_hcd_stop(int index)
{
#if defined(CONFIG_R8A7795)
	const u32 mask = SMSTPCR703 | SMSTPCR702 | SMSTPCR701 | SMSTPCR700;
#else
	const u32 mask = SMSTPCR703 | SMSTPCR702;
#endif
	const u32 base = RCAR_GEN3_USB_BASE(index);
	int ret;

	/* Reset EHCI */
	setbits_le32((uintptr_t)(base + EHCI_USBCMD), CMD_RESET);
	ret = wait_for_bit("ehci-rcar", (void *)(uintptr_t)base + EHCI_USBCMD,
			   CMD_RESET, false, 10, true);
	if (ret) {
		printf("ehci-rcar: reset failed (index=%i, ret=%i).\n",
		       index, ret);
	}

	setbits_le32(SMSTPCR7, BIT(3 - index));

	if ((readl(SMSTPCR7) & mask) == mask)
		setbits_le32(SMSTPCR7, SMSTPCR704);

	return 0;
}

int ehci_hcd_init(int index, enum usb_init_type init,
		  struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	const void __iomem *base =
		(void __iomem *)(uintptr_t)RCAR_GEN3_USB_BASE(index);
	struct usb_ehci *ehci = (struct usb_ehci *)(uintptr_t)base;

	clrbits_le32(SMSTPCR7, BIT(3 - index));
	clrbits_le32(SMSTPCR7, SMSTPCR704);

	*hccr = (struct ehci_hccr *)((uintptr_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uintptr_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	/* Enable interrupt */
	setbits_le32(base + AHB_INT_ENABLE, USBH_INTBEN | USBH_INTAEN);
	writel(0x014e029b, base + CORE_SPD_RSM_TIMSET);
	writel(0x000209ab, base + CORE_OC_TIMSET);

	/* Choice USB0SEL */
	clrsetbits_le32(REG_UGCTRL2, USB0SEL, USB0SEL_EHCI);

	/* Clock & Reset */
	clrbits_le32(base + AHB_USBCTR, AHB_PLL_RST);

	/* low power status */
	clrsetbits_le16(REG_LPSTS, SUSPM, SUSPM_NORMAL);

	return 0;
}

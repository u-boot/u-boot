/*
 * Faraday USB 2.0 EHCI Controller
 *
 * (C) Copyright 2010 Faraday Technology
 * Dante Su <dantesu@faraday-tech.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include <usb/fusbh200.h>
#include <usb/fotg210.h>

#include "ehci.h"

#ifndef CONFIG_USB_EHCI_BASE_LIST
#define CONFIG_USB_EHCI_BASE_LIST	{ CONFIG_USB_EHCI_BASE }
#endif

union ehci_faraday_regs {
	struct fusbh200_regs usb;
	struct fotg210_regs  otg;
};

static inline int ehci_is_fotg2xx(union ehci_faraday_regs *regs)
{
	return !readl(&regs->usb.easstr);
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, struct ehci_hccr **ret_hccr,
		struct ehci_hcor **ret_hcor)
{
	struct ehci_hccr *hccr;
	struct ehci_hcor *hcor;
	union ehci_faraday_regs *regs;
	uint32_t base_list[] = CONFIG_USB_EHCI_BASE_LIST;

	if (index < 0 || index >= ARRAY_SIZE(base_list))
		return -1;
	regs = (void __iomem *)base_list[index];
	hccr = (struct ehci_hccr *)&regs->usb.hccr;
	hcor = (struct ehci_hcor *)&regs->usb.hcor;

	if (ehci_is_fotg2xx(regs)) {
		/* A-device bus reset */
		/* ... Power off A-device */
		setbits_le32(&regs->otg.otgcsr, OTGCSR_A_BUSDROP);
		/* ... Drop vbus and bus traffic */
		clrbits_le32(&regs->otg.otgcsr, OTGCSR_A_BUSREQ);
		mdelay(1);
		/* ... Power on A-device */
		clrbits_le32(&regs->otg.otgcsr, OTGCSR_A_BUSDROP);
		/* ... Drive vbus and bus traffic */
		setbits_le32(&regs->otg.otgcsr, OTGCSR_A_BUSREQ);
		mdelay(1);
		/* Disable OTG & DEV interrupts, triggered at level-high */
		writel(IMR_IRQLH | IMR_OTG | IMR_DEV, &regs->otg.imr);
		/* Clear all interrupt status */
		writel(ISR_HOST | ISR_OTG | ISR_DEV, &regs->otg.isr);
	} else {
		/* Interrupt=level-high */
		setbits_le32(&regs->usb.bmcsr, BMCSR_IRQLH);
		/* VBUS on */
		clrbits_le32(&regs->usb.bmcsr, BMCSR_VBUS_OFF);
		/* Disable all interrupts */
		writel(0x00, &regs->usb.bmier);
		writel(0x1f, &regs->usb.bmisr);
	}

	*ret_hccr = hccr;
	*ret_hcor = hcor;

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	return 0;
}

/*
 * This ehci_set_usbmode() overrides the weak function
 * in "ehci-hcd.c".
 */
void ehci_set_usbmode(int index)
{
	/* nothing needs to be done */
}

/*
 * This ehci_get_port_speed() overrides the weak function
 * in "ehci-hcd.c".
 */
int ehci_get_port_speed(struct ehci_hcor *hcor, uint32_t reg)
{
	int spd, ret = PORTSC_PSPD_HS;
	union ehci_faraday_regs *regs = (void __iomem *)((ulong)hcor - 0x10);

	if (ehci_is_fotg2xx(regs))
		spd = OTGCSR_SPD(readl(&regs->otg.otgcsr));
	else
		spd = BMCSR_SPD(readl(&regs->usb.bmcsr));

	switch (spd) {
	case 0:    /* full speed */
		ret = PORTSC_PSPD_FS;
		break;
	case 1:    /* low  speed */
		ret = PORTSC_PSPD_LS;
		break;
	case 2:    /* high speed */
		ret = PORTSC_PSPD_HS;
		break;
	default:
		printf("ehci-faraday: invalid device speed\n");
		break;
	}

	return ret;
}

/*
 * This ehci_get_portsc_register() overrides the weak function
 * in "ehci-hcd.c".
 */
uint32_t *ehci_get_portsc_register(struct ehci_hcor *hcor, int port)
{
	/* Faraday EHCI has one and only one portsc register */
	if (port) {
		/* Printing the message would cause a scan failure! */
		debug("The request port(%d) is not configured\n", port);
		return NULL;
	}

	/* Faraday EHCI PORTSC register offset is 0x20 from hcor */
	return (uint32_t *)((uint8_t *)hcor + 0x20);
}

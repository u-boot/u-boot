/*
 * (C) Copyright 2010, Damien Dusha, <d.dusha@gmail.com>
 *
 * (C) Copyright 2009, Value Team S.p.A.
 * Francesco Rendine, <francesco.rendine@valueteam.com>
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2008, Excito Elektronik i Sk=E5ne AB
 *
 * Author: Tor Krill tor@excito.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <pci.h>
#include <usb.h>
#include <asm/io.h>
#include <usb/ehci-fsl.h>

#include "ehci.h"

static void fsl_setup_phy(volatile struct ehci_hcor *);
static void fsl_platform_set_host_mode(volatile struct usb_ehci *ehci);
static int reset_usb_controller(volatile struct usb_ehci *ehci);
static void usb_platform_dr_init(volatile struct usb_ehci *ehci);

/*
 * Initialize SOC FSL EHCI Controller
 *
 * This code is derived from EHCI FSL USB Linux driver for MPC5121
 *
 */
int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	volatile struct usb_ehci *ehci;

	/* Hook the memory mapped registers for EHCI-Controller */
	ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB_ADDR;
	*hccr = (struct ehci_hccr *)((uint32_t)&(ehci->caplength));
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
				HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	/* configure interface for UTMI_WIDE */
	usb_platform_dr_init(ehci);

	/* Init Phy USB0 to UTMI+ */
	fsl_setup_phy(*hcor);

	/* Set to host mode */
	fsl_platform_set_host_mode(ehci);

	/*
	 * Setting the burst size seems to be required to prevent the
	 * USB from hanging when communicating with certain USB Mass
	 * storage devices. This was determined by analysing the
	 * EHCI registers under Linux vs U-Boot and burstsize was the
	 * major non-interrupt related difference between the two
	 * implementations.
	 *
	 * Some USB sticks behave better than others. In particular,
	 * the following USB stick is especially problematic:
	 * 0930:6545 Toshiba Corp
	 *
	 * The burstsize is set here to match the Linux implementation.
	 */
	out_be32(&ehci->burstsize, FSL_EHCI_TXPBURST(8) |
				   FSL_EHCI_RXPBURST(8));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	volatile struct usb_ehci *ehci;
	int exit_status = 0;

	/* Reset the USB controller */
	ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB_ADDR;
	exit_status = reset_usb_controller(ehci);

	return exit_status;
}

static int reset_usb_controller(volatile struct usb_ehci *ehci)
{
	unsigned int i;

	/* Command a reset of the USB Controller */
	out_be32(&(ehci->usbcmd), EHCI_FSL_USBCMD_RST);

	/* Wait for the reset process to finish */
	for (i = 65535 ; i > 0 ; i--) {
		/*
		 * The host will set this bit to zero once the
		 * reset process is complete
		 */
		if ((in_be32(&(ehci->usbcmd)) & EHCI_FSL_USBCMD_RST) == 0)
			return 0;
	}

	/* Hub did not reset in time */
	return -1;
}

static void fsl_setup_phy(volatile struct ehci_hcor *hcor)
{
	uint32_t portsc;

	portsc  = ehci_readl(&hcor->or_portsc[0]);
	portsc &= ~(PORT_PTS_MSK | PORT_PTS_PTW);

	/* Enable the phy mode to UTMI Wide */
	portsc |= PORT_PTS_PTW;
	portsc |= PORT_PTS_UTMI;

	ehci_writel(&hcor->or_portsc[0], portsc);
}

static void fsl_platform_set_host_mode(volatile struct usb_ehci *ehci)
{
	uint32_t temp;

	temp  = in_le32(&ehci->usbmode);
	temp |= CM_HOST | ES_BE;
	out_le32(&ehci->usbmode, temp);
}

static void usb_platform_dr_init(volatile struct usb_ehci *ehci)
{
	/* Configure interface for UTMI_WIDE */
	out_be32(&ehci->isiphyctrl, PHYCTRL_PHYE | PHYCTRL_PXE);
	out_be32(&ehci->usbgenctrl, GC_PPP | GC_PFP );
}

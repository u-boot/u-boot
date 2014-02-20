/*
 * (C) Copyright 2014, Xilinx, Inc
 *
 * USB Low level initialization(Specific to zynq)
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <usb.h>
#include <usb/ehci-fsl.h>
#include <usb/ulpi.h>

#include "ehci.h"

#define ZYNQ_USB_USBCMD_RST			0x0000002
#define ZYNQ_USB_USBCMD_STOP			0x0000000
#define ZYNQ_USB_NUM_MIO			12

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index,  enum usb_init_type init, struct ehci_hccr **hccr,
		  struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;
	struct ulpi_viewport ulpi_vp;
	int ret, mio_usb;
	/* Used for writing the ULPI data address */
	struct ulpi_regs *ulpi = (struct ulpi_regs *)0;

	if (!index) {
		mio_usb = zynq_slcr_get_mio_pin_status("usb0");
		if (mio_usb != ZYNQ_USB_NUM_MIO) {
			printf("usb0 wrong num MIO: %d, Index %d\n", mio_usb,
			       index);
			return -1;
		}
		ehci = (struct usb_ehci *)ZYNQ_USB_BASEADDR0;
	} else {
		mio_usb = zynq_slcr_get_mio_pin_status("usb1");
		if (mio_usb != ZYNQ_USB_NUM_MIO) {
			printf("usb1 wrong num MIO: %d, Index %d\n", mio_usb,
			       index);
			return -1;
		}
		ehci = (struct usb_ehci *)ZYNQ_USB_BASEADDR1;
	}

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	ulpi_vp.viewport_addr = (u32)&ehci->ulpi_viewpoint;
	ulpi_vp.port_num = 0;

	ret = ulpi_init(&ulpi_vp);
	if (ret) {
		puts("zynq ULPI viewport init failed\n");
		return -1;
	}

	/* ULPI set flags */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl,
		   ULPI_OTG_DP_PULLDOWN | ULPI_OTG_DM_PULLDOWN |
		   ULPI_OTG_EXTVBUSIND);
	ulpi_write(&ulpi_vp, &ulpi->function_ctrl,
		   ULPI_FC_FULL_SPEED | ULPI_FC_OPMODE_NORMAL |
		   ULPI_FC_SUSPENDM);
	ulpi_write(&ulpi_vp, &ulpi->iface_ctrl, 0);

	/* Set VBus */
	ulpi_write(&ulpi_vp, &ulpi->otg_ctrl_set,
		   ULPI_OTG_DRVVBUS | ULPI_OTG_DRVVBUS_EXT);

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(int index)
{
	struct usb_ehci *ehci;

	if (!index)
		ehci = (struct usb_ehci *)ZYNQ_USB_BASEADDR0;
	else
		ehci = (struct usb_ehci *)ZYNQ_USB_BASEADDR1;

	/* Stop controller */
	writel(ZYNQ_USB_USBCMD_STOP, &ehci->usbcmd);
	udelay(1000);

	/* Initiate controller reset */
	writel(ZYNQ_USB_USBCMD_RST, &ehci->usbcmd);

	return 0;
}

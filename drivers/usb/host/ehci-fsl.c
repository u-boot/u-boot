/*
 * (C) Copyright 2009, 2011 Freescale Semiconductor, Inc.
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
#include <hwconfig.h>
#include <asm/fsl_errata.h>

#include "ehci.h"

static void set_txfifothresh(struct usb_ehci *, u32);

/* Check USB PHY clock valid */
static int usb_phy_clk_valid(struct usb_ehci *ehci)
{
	if (!((in_be32(&ehci->control) & PHY_CLK_VALID) ||
			in_be32(&ehci->prictrl))) {
		printf("USB PHY clock invalid!\n");
		return 0;
	} else {
		return 1;
	}
}

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 *
 * Excerpts from linux ehci fsl driver.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci = NULL;
	const char *phy_type = NULL;
	size_t len;
	char current_usb_controller[5];
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
	char usb_phy[5];

	usb_phy[0] = '\0';
#endif
	if (has_erratum_a007075()) {
		/*
		 * A 5ms delay is needed after applying soft-reset to the
		 * controller to let external ULPI phy come out of reset.
		 * This delay needs to be added before re-initializing
		 * the controller after soft-resetting completes
		 */
		mdelay(5);
	}
	memset(current_usb_controller, '\0', 5);
	snprintf(current_usb_controller, 4, "usb%d", index+1);

	switch (index) {
	case 0:
		ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB1_ADDR;
		break;
	case 1:
		ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB2_ADDR;
		break;
	default:
		printf("ERROR: wrong controller index!!\n");
		break;
	};

	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	/* Set to Host mode */
	setbits_le32(&ehci->usbmode, CM_HOST);

	out_be32(&ehci->snoop1, SNOOP_SIZE_2GB);
	out_be32(&ehci->snoop2, 0x80000000 | SNOOP_SIZE_2GB);

	/* Init phy */
	if (hwconfig_sub(current_usb_controller, "phy_type"))
		phy_type = hwconfig_subarg(current_usb_controller,
				"phy_type", &len);
	else
		phy_type = getenv("usb_phy_type");

	if (!phy_type) {
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
		/* if none specified assume internal UTMI */
		strcpy(usb_phy, "utmi");
		phy_type = usb_phy;
#else
		printf("WARNING: USB phy type not defined !!\n");
		return -1;
#endif
	}

	if (!strncmp(phy_type, "utmi", 4)) {
#if defined(CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY)
		setbits_be32(&ehci->control, PHY_CLK_SEL_UTMI);
		setbits_be32(&ehci->control, UTMI_PHY_EN);
		udelay(1000); /* delay required for PHY Clk to appear */
#endif
		out_le32(&(*hcor)->or_portsc[0], PORT_PTS_UTMI);
		setbits_be32(&ehci->control, USB_EN);
	} else {
		setbits_be32(&ehci->control, PHY_CLK_SEL_ULPI);
		clrsetbits_be32(&ehci->control, UTMI_PHY_EN, USB_EN);
		udelay(1000); /* delay required for PHY Clk to appear */
		if (!usb_phy_clk_valid(ehci))
			return -EINVAL;
		out_le32(&(*hcor)->or_portsc[0], PORT_PTS_ULPI);
	}

	out_be32(&ehci->prictrl, 0x0000000c);
	out_be32(&ehci->age_cnt_limit, 0x00000040);
	out_be32(&ehci->sictrl, 0x00000001);

	in_le32(&ehci->usbmode);

	if (SVR_SOC_VER(get_svr()) == SVR_T4240 &&
	    IS_SVR_REV(get_svr(), 2, 0))
		set_txfifothresh(ehci, TXFIFOTHRESH);

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
 * Setting the value of TXFIFO_THRESH field in TXFILLTUNING register
 * to counter DDR latencies in writing data into Tx buffer.
 * This prevents Tx buffer from getting underrun
 */
static void set_txfifothresh(struct usb_ehci *ehci, u32 txfifo_thresh)
{
	u32 cmd;
	cmd = ehci_readl(&ehci->txfilltuning);
	cmd &= ~TXFIFO_THRESH_MASK;
	cmd |= TXFIFO_THRESH(txfifo_thresh);
	ehci_writel(&ehci->txfilltuning, cmd);
}

/*
 * (C) Copyright 2009, 2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2008, Excito Elektronik i Sk=E5ne AB
 *
 * Author: Tor Krill tor@excito.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <pci.h>
#include <usb.h>
#include <asm/io.h>
#include <usb/ehci-fsl.h>
#include <hwconfig.h>

#include "ehci.h"

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
int ehci_hcd_init(int index, struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	struct usb_ehci *ehci;
	const char *phy_type = NULL;
	size_t len;
#ifdef CONFIG_SYS_FSL_USB_INTERNAL_UTMI_PHY
	char usb_phy[5];

	usb_phy[0] = '\0';
#endif

	ehci = (struct usb_ehci *)CONFIG_SYS_FSL_USB_ADDR;
	*hccr = (struct ehci_hccr *)((uint32_t)&ehci->caplength);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	/* Set to Host mode */
	setbits_le32(&ehci->usbmode, CM_HOST);

	out_be32(&ehci->snoop1, SNOOP_SIZE_2GB);
	out_be32(&ehci->snoop2, 0x80000000 | SNOOP_SIZE_2GB);

	/* Init phy */
	if (hwconfig_sub("usb1", "phy_type"))
		phy_type = hwconfig_subarg("usb1", "phy_type", &len);
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

	if (!strcmp(phy_type, "utmi")) {
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

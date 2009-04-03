/*
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
#include <mpc83xx.h>
#include <asm/io.h>
#include <asm/bitops.h>

#include "ehci.h"
#include "ehci-fsl.h"
#include "ehci-core.h"

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 *
 * Excerpts from linux ehci fsl driver.
 */
int ehci_hcd_init(void)
{
	volatile immap_t *im = (immap_t *)CONFIG_SYS_IMMR;
	uint32_t addr, temp;

	addr = (uint32_t)&(im->usb[0]);
	hccr = (struct ehci_hccr *)(addr + FSL_SKIP_PCI);
	hcor = (struct ehci_hcor *)((uint32_t) hccr +
			HC_LENGTH(ehci_readl(&hccr->cr_capbase)));

	/* Configure clock */
	clrsetbits_be32(&(im->clk.sccr), MPC83XX_SCCR_USB_MASK,
			MPC83XX_SCCR_USB_DRCM_11);

	/* Confgure interface. */
	temp = in_be32((void *)(addr + FSL_SOC_USB_CTRL));
	out_be32((void *)(addr + FSL_SOC_USB_CTRL), temp
		 | REFSEL_16MHZ | UTMI_PHY_EN);

	/* Wait for clock to stabilize */
	do {
		temp = in_be32((void *)(addr + FSL_SOC_USB_CTRL));
		udelay(1000);
	} while (!(temp & PHY_CLK_VALID));

	/* Set to Host mode */
	temp = in_le32((void *)(addr + FSL_SOC_USB_USBMODE));
	out_le32((void *)(addr + FSL_SOC_USB_USBMODE), temp | CM_HOST);

	out_be32((void *)(addr + FSL_SOC_USB_SNOOP1), SNOOP_SIZE_2GB);
	out_be32((void *)(addr + FSL_SOC_USB_SNOOP2),
		 0x80000000 | SNOOP_SIZE_2GB);

	/* Init phy */
	/* TODO: handle different phys? */
	out_le32(&(hcor->or_portsc[0]), PORT_PTS_UTMI);

	/* Enable interface. */
	temp = in_be32((void *)(addr + FSL_SOC_USB_CTRL));
	out_be32((void *)(addr + FSL_SOC_USB_CTRL), temp | USB_EN);

	out_be32((void *)(addr + FSL_SOC_USB_PRICTRL), 0x0000000c);
	out_be32((void *)(addr + FSL_SOC_USB_AGECNTTHRSH), 0x00000040);
	out_be32((void *)(addr + FSL_SOC_USB_SICTRL), 0x00000001);

	/* Enable interface. */
	temp = in_be32((void *)(addr + FSL_SOC_USB_CTRL));
	out_be32((void *)(addr + FSL_SOC_USB_CTRL), temp | USB_EN);

	temp = in_le32((void *)(addr + FSL_SOC_USB_USBMODE));

	return 0;
}

/*
 * Destroy the appropriate control structures corresponding
 * the the EHCI host controller.
 */
int ehci_hcd_stop(void)
{
	return 0;
}

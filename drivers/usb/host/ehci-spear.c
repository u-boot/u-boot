/*
 * (C) Copyright 2010
 * Armando Visconti, ST Micoelectronics, <armando.visconti@st.com>.
 *
 * (C) Copyright 2009
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <usb.h>
#include "ehci.h"
#include <asm/arch/hardware.h>


/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	*hccr = (struct ehci_hccr *)(CONFIG_SYS_UHC0_EHCI_BASE + 0x100);
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr
			+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("SPEAr-ehci: init hccr %x and hcor %x hc_length %d\n",
		(uint32_t)*hccr, (uint32_t)*hcor,
		(uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

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

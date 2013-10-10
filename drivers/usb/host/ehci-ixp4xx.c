/*
 * (C) Copyright 2008, Michael Trimarchi <trimarchimichael@yahoo.it>
 *
 * Author: Michael Trimarchi <trimarchimichael@yahoo.it>
 * This code is based on ehci freescale driver
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <common.h>
#include <usb.h>

#include "ehci.h"
/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	*hccr = (struct ehci_hccr *)(0xcd000100);
	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
			+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	printf("IXP4XX init hccr %x and hcor %x hc_length %d\n",
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

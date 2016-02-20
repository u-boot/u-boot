/*
 * (C) Copyright 2012
 * Atmel Semiconductor <www.atmel.com>
 * Written-by: Bo Shen <voice.shen@atmel.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <asm/io.h>
#include <asm/arch/clk.h>

#include "ehci.h"

int ehci_hcd_init(int index, enum usb_init_type init,
		struct ehci_hccr **hccr, struct ehci_hcor **hcor)
{
	/* Enable UTMI PLL */
	if (at91_upll_clk_enable())
		return -1;

	/* Enable USB Host clock */
	at91_periph_clk_enable(ATMEL_ID_UHPHS);

	*hccr = (struct ehci_hccr *)ATMEL_BASE_EHCI;
	*hcor = (struct ehci_hcor *)((uint32_t)*hccr +
			HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

int ehci_hcd_stop(int index)
{
	/* Disable USB Host Clock */
	at91_periph_clk_disable(ATMEL_ID_UHPHS);

	/* Disable UTMI PLL */
	if (at91_upll_clk_disable())
		return -1;

	return 0;
}

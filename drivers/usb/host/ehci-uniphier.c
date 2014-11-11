/*
 * Copyright (C) 2014 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <usb.h>
#include <asm/arch/ehci-uniphier.h>
#include "ehci.h"

/*
 * Create the appropriate control structures to manage
 * a new EHCI host controller.
 */
int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		  struct ehci_hcor **hcor)
{
	struct ehci_hccr *cr;
	struct ehci_hcor *or;

	uniphier_ehci_reset(index, 0);

	cr = (struct ehci_hccr *)(uniphier_ehci_platdata[index].base);
	or = (void *)cr + HC_LENGTH(ehci_readl(&cr->cr_capbase));

	*hccr = cr;
	*hcor = or;

	return 0;
}

int ehci_hcd_stop(int index)
{
	uniphier_ehci_reset(index, 1);

	return 0;
}

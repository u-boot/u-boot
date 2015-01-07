/*
 * Copyright (C) 2014 Roman Byshko
 *
 * Roman Byshko <rbyshko@gmail.com>
 *
 * Based on code from
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/arch/usbc.h>
#include <common.h>
#include "ehci.h"

int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		struct ehci_hcor **hcor)
{
	int err;

	err = sunxi_usbc_request_resources(index + 1);
	if (err)
		return err;

	sunxi_usbc_enable(index + 1);
	sunxi_usbc_vbus_enable(index + 1);

	*hccr = sunxi_usbc_get_io_base(index + 1);

	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("sunxi-ehci: init hccr %x and hcor %x hc_length %d\n",
	      (uint32_t)*hccr, (uint32_t)*hcor,
	      (uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

int ehci_hcd_stop(int index)
{
	sunxi_usbc_vbus_disable(index + 1);
	sunxi_usbc_disable(index + 1);

	return sunxi_usbc_free_resources(index + 1);
}

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

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/usbc.h>
#include <asm/io.h>
#include "ehci.h"

int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		struct ehci_hcor **hcor)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int ahb_gate_offset, err;

	err = sunxi_usbc_request_resources(index + 1);
	if (err)
		return err;

	ahb_gate_offset = index ? AHB_GATE_OFFSET_USB_EHCI1 :
				  AHB_GATE_OFFSET_USB_EHCI0;
	setbits_le32(&ccm->ahb_gate0, 1 << ahb_gate_offset);
#ifdef CONFIG_SUNXI_GEN_SUN6I
	setbits_le32(&ccm->ahb_reset0_cfg, 1 << ahb_gate_offset);
#endif

	sunxi_usbc_enable(index + 1);
	sunxi_usbc_vbus_enable(index + 1);

	if (index == 0)
		*hccr = (void *)SUNXI_USB1_BASE;
	else
		*hccr = (void *)SUNXI_USB2_BASE;

	*hcor = (struct ehci_hcor *)((uint32_t) *hccr
				+ HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	debug("sunxi-ehci: init hccr %x and hcor %x hc_length %d\n",
	      (uint32_t)*hccr, (uint32_t)*hcor,
	      (uint32_t)HC_LENGTH(ehci_readl(&(*hccr)->cr_capbase)));

	return 0;
}

int ehci_hcd_stop(int index)
{
	struct sunxi_ccm_reg *ccm = (struct sunxi_ccm_reg *)SUNXI_CCM_BASE;
	int ahb_gate_offset;

	sunxi_usbc_vbus_disable(index + 1);
	sunxi_usbc_disable(index + 1);

	ahb_gate_offset = index ? AHB_GATE_OFFSET_USB_EHCI1 :
				  AHB_GATE_OFFSET_USB_EHCI0;
#ifdef CONFIG_SUNXI_GEN_SUN6I
	clrbits_le32(&ccm->ahb_reset0_cfg, 1 << ahb_gate_offset);
#endif
	clrbits_le32(&ccm->ahb_gate0, 1 << ahb_gate_offset);

	return sunxi_usbc_free_resources(index + 1);
}

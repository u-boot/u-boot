/*
 * Copyright (C) 2014 Panasonic Corporation
 * Copyright (C) 2015 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <asm/io.h>
#include <usb.h>
#include <mach/mio-regs.h>
#include <fdtdec.h>
#include "ehci.h"

DECLARE_GLOBAL_DATA_PTR;

#define FDT		gd->fdt_blob
#define COMPAT		"socionext,uniphier-ehci"

static int get_uniphier_ehci_base(int index, struct ehci_hccr **base)
{
	int offset;

	for (offset = fdt_node_offset_by_compatible(FDT, 0, COMPAT);
	     offset >= 0;
	     offset = fdt_node_offset_by_compatible(FDT, offset, COMPAT)) {
		if (index == 0) {
			*base = (struct ehci_hccr *)
					fdtdec_get_addr(FDT, offset, "reg");
			return 0;
		}
		index--;
	}

	return -ENODEV; /* not found */
}

static void uniphier_ehci_reset(int index, int on)
{
	u32 tmp;

	tmp = readl(MIO_USB_RSTCTRL(index));
	if (on)
		tmp &= ~MIO_USB_RSTCTRL_XRST;
	else
		tmp |= MIO_USB_RSTCTRL_XRST;
	writel(tmp, MIO_USB_RSTCTRL(index));
}

int ehci_hcd_init(int index, enum usb_init_type init, struct ehci_hccr **hccr,
		  struct ehci_hcor **hcor)
{
	int ret;
	struct ehci_hccr *cr;
	struct ehci_hcor *or;

	uniphier_ehci_reset(index, 0);

	ret = get_uniphier_ehci_base(index, &cr);
	if (ret < 0)
		return ret;
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

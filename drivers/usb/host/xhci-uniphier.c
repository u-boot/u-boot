/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/err.h>
#include <linux/io.h>
#include <usb.h>
#include <fdtdec.h>
#include "xhci.h"

static int get_uniphier_xhci_base(int index, struct xhci_hccr **base)
{
	DECLARE_GLOBAL_DATA_PTR;
	int node_list[2];
	fdt_addr_t addr;
	int count;

	count = fdtdec_find_aliases_for_id(gd->fdt_blob, "usb",
					   COMPAT_SOCIONEXT_XHCI, node_list,
					   ARRAY_SIZE(node_list));

	if (index >= count)
		return -ENODEV;

	addr = fdtdec_get_addr(gd->fdt_blob, node_list[index], "reg");
	if (addr == FDT_ADDR_T_NONE)
		return -ENODEV;

	*base = (struct xhci_hccr *)addr;

	return 0;
}

#define USB3_RST_CTRL		0x00100040
#define IOMMU_RST_N		(1 << 5)
#define LINK_RST_N		(1 << 4)

static void uniphier_xhci_reset(void __iomem *base, int on)
{
	u32 tmp;

	tmp = readl(base + USB3_RST_CTRL);

	if (on)
		tmp &= ~(IOMMU_RST_N | LINK_RST_N);
	else
		tmp |= IOMMU_RST_N | LINK_RST_N;

	writel(tmp, base + USB3_RST_CTRL);
}

int xhci_hcd_init(int index, struct xhci_hccr **hccr, struct xhci_hcor **hcor)
{
	int ret;
	struct xhci_hccr *cr;
	struct xhci_hcor *or;

	ret = get_uniphier_xhci_base(index, &cr);
	if (ret < 0)
		return ret;

	uniphier_xhci_reset(cr, 0);

	or = (void *)cr + HC_LENGTH(xhci_readl(&cr->cr_capbase));

	*hccr = cr;
	*hcor = or;

	return 0;
}

void xhci_hcd_stop(int index)
{
	int ret;
	struct xhci_hccr *cr;

	ret = get_uniphier_xhci_base(index, &cr);
	if (ret < 0)
		return;

	uniphier_xhci_reset(cr, 1);
}

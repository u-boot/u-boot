/*
 * Copyright (C) 2014-2015 Panasonic Corporation
 *   Author: Masahiro Yamada <yamada.m@jp.panasonic.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __PLAT_UNIPHIER_EHCI_H
#define __PLAT_UNIPHIER_EHCI_H

#include <linux/types.h>
#include <asm/io.h>
#include "mio-regs.h"

static inline void uniphier_ehci_reset(int index, int on)
{
	u32 tmp;

	tmp = readl(MIO_USB_RSTCTRL(index));
	if (on)
		tmp &= ~MIO_USB_RSTCTRL_XRST;
	else
		tmp |= MIO_USB_RSTCTRL_XRST;
	writel(tmp, MIO_USB_RSTCTRL(index));
}

#endif /* __PLAT_UNIPHIER_EHCI_H */

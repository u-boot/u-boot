/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/sg-regs.h>

void pin_init(void)
{
#ifdef CONFIG_USB_EHCI_UNIPHIER
	sg_set_pinsel(13, 0);	/* USB0OC */
	sg_set_pinsel(14, 1);	/* USB0VBUS */

	sg_set_pinsel(15, 0);	/* USB1OC */
	sg_set_pinsel(16, 1);	/* USB1VBUS */

	sg_set_pinsel(17, 0);	/* USB2OC */
	sg_set_pinsel(18, 1);	/* USB2VBUS */

	sg_set_pinsel(19, 0);	/* USB3OC */
	sg_set_pinsel(20, 1);	/* USB3VBUS */
#endif
}

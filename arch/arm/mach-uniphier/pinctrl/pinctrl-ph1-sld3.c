/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/init.h>
#include <mach/sg-regs.h>

void ph1_sld3_pin_init(void)
{
#ifdef CONFIG_USB_EHCI_UNIPHIER
	sg_set_pinsel(13, 0, 4, 4);	/* USB0OC */
	sg_set_pinsel(14, 1, 4, 4);	/* USB0VBUS */

	sg_set_pinsel(15, 0, 4, 4);	/* USB1OC */
	sg_set_pinsel(16, 1, 4, 4);	/* USB1VBUS */

	sg_set_pinsel(17, 0, 4, 4);	/* USB2OC */
	sg_set_pinsel(18, 1, 4, 4);	/* USB2VBUS */

	sg_set_pinsel(19, 0, 4, 4);	/* USB3OC */
	sg_set_pinsel(20, 1, 4, 4);	/* USB3VBUS */
#endif
}

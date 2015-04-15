/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2015      Socionext Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <asm/io.h>
#include <mach/sg-regs.h>

void pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(40, 0);	/* NFD0   -> NFD0 */
	sg_set_pinsel(41, 0);	/* NFD1   -> NFD1 */
	sg_set_pinsel(42, 0);	/* NFD2   -> NFD2 */
	sg_set_pinsel(43, 0);	/* NFD3   -> NFD3 */
	sg_set_pinsel(44, 0);	/* NFD4   -> NFD4 */
	sg_set_pinsel(45, 0);	/* NFD5   -> NFD5 */
	sg_set_pinsel(46, 0);	/* NFD6   -> NFD6 */
	sg_set_pinsel(47, 0);	/* NFD7   -> NFD7 */
	sg_set_pinsel(48, 0);	/* NFALE  -> NFALE */
	sg_set_pinsel(49, 0);	/* NFCLE  -> NFCLE */
	sg_set_pinsel(50, 0);	/* XNFRE  -> XNFRE */
	sg_set_pinsel(51, 0);	/* XNFWE  -> XNFWE */
	sg_set_pinsel(52, 0);	/* XNFWP  -> XNFWP */
	sg_set_pinsel(53, 0);	/* XNFCE0 -> XNFCE0 */
	sg_set_pinsel(54, 0);	/* NRYBY0 -> NRYBY0 */
#endif

#ifdef CONFIG_USB_XHCI_UNIPHIER
	sg_set_pinsel(180, 0);	/* USB0VBUS -> USB0VBUS */
	sg_set_pinsel(181, 0);	/* USB0OD   -> USB0OD */
	sg_set_pinsel(182, 0);	/* USB1VBUS -> USB1VBUS */
	sg_set_pinsel(183, 0);	/* USB1OD   -> USB1OD */
#endif

#ifdef CONFIG_USB_EHCI_UNIPHIER
	sg_set_pinsel(184, 0);	/* USB2VBUS -> USB2VBUS */
	sg_set_pinsel(185, 0);	/* USB2OD   -> USB2OD */
	sg_set_pinsel(187, 0);	/* USB3VBUS -> USB3VBUS */
	sg_set_pinsel(188, 0);	/* USB3OD   -> USB3OD */
#endif

	writel(1, SG_LOADPINCTRL);
}

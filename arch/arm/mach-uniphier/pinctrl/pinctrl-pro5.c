/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

void ph1_pro5_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(19, 0, 4, 8);	/* XNFRE  -> XNFRE */
	sg_set_pinsel(20, 0, 4, 8);	/* XNFWE  -> XNFWE */
	sg_set_pinsel(21, 0, 4, 8);	/* NFALE  -> NFALE */
	sg_set_pinsel(22, 0, 4, 8);	/* NFCLE  -> NFCLE */
	sg_set_pinsel(23, 0, 4, 8);	/* XNFWP  -> XNFWP */
	sg_set_pinsel(24, 0, 4, 8);	/* XNFCE0 -> XNFCE0 */
	sg_set_pinsel(25, 0, 4, 8);	/* NRYBY0 -> NRYBY0 */
	sg_set_pinsel(26, 0, 4, 8);	/* XNFCE1 -> XNFCE1 */
	sg_set_pinsel(27, 0, 4, 8);	/* NRYBY1 -> NRYBY1 */
	sg_set_pinsel(28, 0, 4, 8);	/* NFD0   -> NFD0 */
	sg_set_pinsel(29, 0, 4, 8);	/* NFD1   -> NFD1 */
	sg_set_pinsel(30, 0, 4, 8);	/* NFD2   -> NFD2 */
	sg_set_pinsel(31, 0, 4, 8);	/* NFD3   -> NFD3 */
	sg_set_pinsel(32, 0, 4, 8);	/* NFD4   -> NFD4 */
	sg_set_pinsel(33, 0, 4, 8);	/* NFD5   -> NFD5 */
	sg_set_pinsel(34, 0, 4, 8);	/* NFD6   -> NFD6 */
	sg_set_pinsel(35, 0, 4, 8);	/* NFD7   -> NFD7 */
#endif

#ifdef CONFIG_USB_XHCI_UNIPHIER
	sg_set_pinsel(124, 0, 4, 8);	/* USB0VBUS -> USB0VBUS */
	sg_set_pinsel(125, 0, 4, 8);	/* USB0OD   -> USB0OD */
	sg_set_pinsel(126, 0, 4, 8);	/* USB1VBUS -> USB1VBUS */
	sg_set_pinsel(127, 0, 4, 8);	/* USB1OD   -> USB1OD */
#endif

	writel(1, SG_LOADPINCTRL);
}

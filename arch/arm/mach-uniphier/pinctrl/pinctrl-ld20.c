/*
 * Copyright (C) 2016 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

void uniphier_ld20_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(3, 0, 8, 4);	/* XNFWP   -> XNFWP */
	sg_set_pinsel(4, 0, 8, 4);	/* XNFCE0  -> XNFCE0 */
	sg_set_pinsel(5, 0, 8, 4);	/* NFRYBY0 -> NFRYBY0 */
	sg_set_pinsel(6, 0, 8, 4);	/* XNFRE   -> XNFRE */
	sg_set_pinsel(7, 0, 8, 4);	/* XNFWE   -> XNFWE */
	sg_set_pinsel(8, 0, 8, 4);	/* NFALE   -> NFALE */
	sg_set_pinsel(9, 0, 8, 4);	/* NFCLE   -> NFCLE */
	sg_set_pinsel(10, 0, 8, 4);	/* NFD0    -> NFD0 */
	sg_set_pinsel(11, 0, 8, 4);	/* NFD1    -> NFD1 */
	sg_set_pinsel(12, 0, 8, 4);	/* NFD2    -> NFD2 */
	sg_set_pinsel(13, 0, 8, 4);	/* NFD3    -> NFD3 */
	sg_set_pinsel(14, 0, 8, 4);	/* NFD4    -> NFD4 */
	sg_set_pinsel(15, 0, 8, 4);	/* NFD5    -> NFD5 */
	sg_set_pinsel(16, 0, 8, 4);	/* NFD6    -> NFD6 */
	sg_set_pinsel(17, 0, 8, 4);	/* NFD7    -> NFD7 */
	sg_set_iectrl_range(3, 17);
#endif

#ifdef CONFIG_USB_XHCI_UNIPHIER
	sg_set_pinsel(46, 0, 8, 4);	/* USB0VBUS -> USB0VBUS */
	sg_set_pinsel(47, 0, 8, 4);	/* USB0OD   -> USB0OD */
	sg_set_pinsel(48, 0, 8, 4);	/* USB1VBUS -> USB1VBUS */
	sg_set_pinsel(49, 0, 8, 4);	/* USB1OD   -> USB1OD */
	sg_set_pinsel(50, 0, 8, 4);	/* USB2VBUS -> USB2VBUS */
	sg_set_pinsel(51, 0, 8, 4);	/* USB2OD   -> USB2OD */
	sg_set_pinsel(52, 0, 8, 4);	/* USB3VBUS -> USB3VBUS */
	sg_set_pinsel(53, 0, 8, 4);	/* USB3OD   -> USB3OD */
	sg_set_iectrl_range(46, 53);
#endif

	sg_set_pinsel(149, 14, 8, 4);	/* XIRQ0    -> XIRQ0 */
	sg_set_iectrl(149);
	sg_set_pinsel(153, 14, 8, 4);	/* XIRQ4    -> XIRQ4 */
	sg_set_iectrl(153);
}

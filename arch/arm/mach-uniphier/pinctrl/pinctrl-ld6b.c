/*
 * Copyright (C) 2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

void uniphier_ld6b_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(30, 0, 8, 4);	/* XNFRE  -> XNFRE */
	sg_set_pinsel(31, 0, 8, 4);	/* XNFWE  -> XNFWE */
	sg_set_pinsel(32, 0, 8, 4);	/* NFALE  -> NFALE */
	sg_set_pinsel(33, 0, 8, 4);	/* NFCLE  -> NFCLE */
	sg_set_pinsel(34, 0, 8, 4);	/* XNFWP  -> XNFWP */
	sg_set_pinsel(35, 0, 8, 4);	/* XNFCE0 -> XNFCE0 */
	sg_set_pinsel(36, 0, 8, 4);	/* NRYBY0 -> NRYBY0 */
	sg_set_pinsel(37, 0, 8, 4);	/* XNFCE1 -> NRYBY1 */
	sg_set_pinsel(38, 0, 8, 4);	/* NRYBY1 -> XNFCE1 */
	sg_set_pinsel(39, 0, 8, 4);	/* NFD0   -> NFD0 */
	sg_set_pinsel(40, 0, 8, 4);	/* NFD1   -> NFD1 */
	sg_set_pinsel(41, 0, 8, 4);	/* NFD2   -> NFD2 */
	sg_set_pinsel(42, 0, 8, 4);	/* NFD3   -> NFD3 */
	sg_set_pinsel(43, 0, 8, 4);	/* NFD4   -> NFD4 */
	sg_set_pinsel(44, 0, 8, 4);	/* NFD5   -> NFD5 */
	sg_set_pinsel(45, 0, 8, 4);	/* NFD6   -> NFD6 */
	sg_set_pinsel(46, 0, 8, 4);	/* NFD7   -> NFD7 */
#endif

#ifdef CONFIG_USB_XHCI_UNIPHIER
	sg_set_pinsel(56, 0, 8, 4);	/* USB0VBUS -> USB0VBUS */
	sg_set_pinsel(57, 0, 8, 4);	/* USB0OD   -> USB0OD */
	sg_set_pinsel(58, 0, 8, 4);	/* USB1VBUS -> USB1VBUS */
	sg_set_pinsel(59, 0, 8, 4);	/* USB1OD   -> USB1OD */
	sg_set_pinsel(60, 0, 8, 4);	/* USB2VBUS -> USB2VBUS */
	sg_set_pinsel(61, 0, 8, 4);	/* USB2OD   -> USB2OD */
	sg_set_pinsel(62, 0, 8, 4);	/* USB3VBUS -> USB3VBUS */
	sg_set_pinsel(63, 0, 8, 4);	/* USB3OD   -> USB3OD */
#endif
}

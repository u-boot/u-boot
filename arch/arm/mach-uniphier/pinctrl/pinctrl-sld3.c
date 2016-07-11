/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "../init.h"
#include "../sg-regs.h"

void uniphier_sld3_pin_init(void)
{
#ifdef CONFIG_USB_EHCI
	sg_set_pinsel(13, 0, 4, 4);	/* USB0OC */
	sg_set_pinsel(14, 1, 4, 4);	/* USB0VBUS */

	sg_set_pinsel(15, 0, 4, 4);	/* USB1OC */
	sg_set_pinsel(16, 1, 4, 4);	/* USB1VBUS */

	sg_set_pinsel(17, 0, 4, 4);	/* USB2OC */
	sg_set_pinsel(18, 1, 4, 4);	/* USB2VBUS */

	sg_set_pinsel(19, 0, 4, 4);	/* USB3OC */
	sg_set_pinsel(20, 1, 4, 4);	/* USB3VBUS */
#endif

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(38, 1, 4, 4);	/* NFALE_GB, NFCLE_GB */
	sg_set_pinsel(39, 1, 4, 4);	/* XNFRYBY0_GB */
	sg_set_pinsel(40, 1, 4, 4);	/* XNFCE0_GB, XNFRE_GB, XNFWE_GB, XNFWP_GB */
	sg_set_pinsel(41, 1, 4, 4);	/* XNFRYBY1_GB, XNFCE1_GB */
	sg_set_pinsel(58, 1, 4, 4);	/* NFD[0-3]_GB */
	sg_set_pinsel(59, 1, 4, 4);	/* NFD[4-7]_GB */
#endif

#ifdef CONFIG_MMC_UNIPHIER
	/* eMMC */
	sg_set_pinsel(55, 1, 4, 4);	/* XERST */
	sg_set_pinsel(56, 1, 4, 4);	/* MMCDAT[0-3] */
	sg_set_pinsel(57, 1, 4, 4);	/* MMCDAT[4-7] */
	sg_set_pinsel(60, 1, 4, 4);	/* MMCCLK, MMCCMD */

	/* SD card */
	sg_set_pinsel(42, 1, 4, 4);	/* SD1CLK, SD1CMD, SD1DAT[0-3] */
	sg_set_pinsel(43, 1, 4, 4);	/* SD1CD */
	sg_set_pinsel(44, 1, 4, 4);	/* SD1WP */
	sg_set_pinsel(45, 1, 4, 4);	/* SDVTCG */
#endif
}

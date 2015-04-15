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
	u32 tmp;

	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(158, 0);	/* XNFRE -> XNFRE_GB */
	sg_set_pinsel(159, 0);	/* XNFWE -> XNFWE_GB */
	sg_set_pinsel(160, 0);	/* XFALE -> NFALE_GB */
	sg_set_pinsel(161, 0);	/* XFCLE -> NFCLE_GB */
	sg_set_pinsel(162, 0);	/* XNFWP -> XFNWP_GB */
	sg_set_pinsel(163, 0);	/* XNFCE0 -> XNFCE0_GB */
	sg_set_pinsel(164, 0);	/* NANDRYBY0 -> NANDRYBY0_GB */
	sg_set_pinsel(22, 0);	/* MMCCLK  -> XFNCE1_GB */
	sg_set_pinsel(23, 0);	/* MMCCMD  -> NANDRYBY1_GB */
	sg_set_pinsel(24, 0);	/* MMCDAT0 -> NFD0_GB */
	sg_set_pinsel(25, 0);	/* MMCDAT1 -> NFD1_GB */
	sg_set_pinsel(26, 0);	/* MMCDAT2 -> NFD2_GB */
	sg_set_pinsel(27, 0);	/* MMCDAT3 -> NFD3_GB */
	sg_set_pinsel(28, 0);	/* MMCDAT4 -> NFD4_GB */
	sg_set_pinsel(29, 0);	/* MMCDAT5 -> NFD5_GB */
	sg_set_pinsel(30, 0);	/* MMCDAT6 -> NFD6_GB */
	sg_set_pinsel(31, 0);	/* MMCDAT7 -> NFD7_GB */
#endif

#ifdef CONFIG_USB_EHCI_UNIPHIER
	sg_set_pinsel(53, 0);	/* USB0VBUS -> USB0VBUS */
	sg_set_pinsel(54, 0);	/* USB0OD   -> USB0OD */
	sg_set_pinsel(55, 0);	/* USB1VBUS -> USB1VBUS */
	sg_set_pinsel(56, 0);	/* USB1OD   -> USB1OD */
	/* sg_set_pinsel(67, 23); */ /* PCOE -> USB2VBUS */
	/* sg_set_pinsel(68, 23); */ /* PCWAIT -> USB2OD */
#endif

	tmp = readl(SG_IECTRL);
	tmp |= 0x41;
	writel(tmp, SG_IECTRL);
}

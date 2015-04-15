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

#ifdef CONFIG_SYS_I2C_UNIPHIER
	{
		u32 tmp;
		tmp = readl(SG_IECTRL);
		tmp |= 0xc00; /* enable SCL0, SDA0, SCL1, SDA1 */
		writel(tmp, SG_IECTRL);
	}
#endif

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(15, 0);	/* XNFRE_GB -> XNFRE_GB */
	sg_set_pinsel(16, 0);	/* XNFWE_GB -> XNFWE_GB */
	sg_set_pinsel(17, 0);	/* XFALE_GB -> NFALE_GB */
	sg_set_pinsel(18, 0);	/* XFCLE_GB -> NFCLE_GB */
	sg_set_pinsel(19, 0);	/* XNFWP_GB -> XFNWP_GB */
	sg_set_pinsel(20, 0);	/* XNFCE0_GB -> XNFCE0_GB */
	sg_set_pinsel(21, 0);	/* NANDRYBY0_GB -> NANDRYBY0_GB */
	sg_set_pinsel(22, 0);	/* XFNCE1_GB  -> XFNCE1_GB */
	sg_set_pinsel(23, 0);	/* NANDRYBY1_GB  -> NANDRYBY1_GB */
	sg_set_pinsel(24, 0);	/* NFD0_GB -> NFD0_GB */
	sg_set_pinsel(25, 0);	/* NFD1_GB -> NFD1_GB */
	sg_set_pinsel(26, 0);	/* NFD2_GB -> NFD2_GB */
	sg_set_pinsel(27, 0);	/* NFD3_GB -> NFD3_GB */
	sg_set_pinsel(28, 0);	/* NFD4_GB -> NFD4_GB */
	sg_set_pinsel(29, 0);	/* NFD5_GB -> NFD5_GB */
	sg_set_pinsel(30, 0);	/* NFD6_GB -> NFD6_GB */
	sg_set_pinsel(31, 0);	/* NFD7_GB -> NFD7_GB */
#endif

#ifdef CONFIG_USB_EHCI_UNIPHIER
	sg_set_pinsel(41, 0);	/* USB0VBUS -> USB0VBUS */
	sg_set_pinsel(42, 0);	/* USB0OD   -> USB0OD */
	sg_set_pinsel(43, 0);	/* USB1VBUS -> USB1VBUS */
	sg_set_pinsel(44, 0);	/* USB1OD   -> USB1OD */
	/* sg_set_pinsel(114, 4); */ /* TXD1 -> USB2VBUS (shared with UART) */
	/* sg_set_pinsel(115, 4); */ /* RXD1 -> USB2OD */
#endif
}

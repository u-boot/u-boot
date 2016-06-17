/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>

#include "../init.h"
#include "../sg-regs.h"

void ph1_sld8_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(15, 0, 8, 4);	/* XNFRE_GB -> XNFRE_GB */
	sg_set_pinsel(16, 0, 8, 4);	/* XNFWE_GB -> XNFWE_GB */
	sg_set_pinsel(17, 0, 8, 4);	/* XFALE_GB -> NFALE_GB */
	sg_set_pinsel(18, 0, 8, 4);	/* XFCLE_GB -> NFCLE_GB */
	sg_set_pinsel(19, 0, 8, 4);	/* XNFWP_GB -> XFNWP_GB */
	sg_set_pinsel(20, 0, 8, 4);	/* XNFCE0_GB -> XNFCE0_GB */
	sg_set_pinsel(21, 0, 8, 4);	/* NANDRYBY0_GB -> NANDRYBY0_GB */
	sg_set_pinsel(22, 0, 8, 4);	/* XFNCE1_GB  -> XFNCE1_GB */
	sg_set_pinsel(23, 0, 8, 4);	/* NANDRYBY1_GB  -> NANDRYBY1_GB */
	sg_set_pinsel(24, 0, 8, 4);	/* NFD0_GB -> NFD0_GB */
	sg_set_pinsel(25, 0, 8, 4);	/* NFD1_GB -> NFD1_GB */
	sg_set_pinsel(26, 0, 8, 4);	/* NFD2_GB -> NFD2_GB */
	sg_set_pinsel(27, 0, 8, 4);	/* NFD3_GB -> NFD3_GB */
	sg_set_pinsel(28, 0, 8, 4);	/* NFD4_GB -> NFD4_GB */
	sg_set_pinsel(29, 0, 8, 4);	/* NFD5_GB -> NFD5_GB */
	sg_set_pinsel(30, 0, 8, 4);	/* NFD6_GB -> NFD6_GB */
	sg_set_pinsel(31, 0, 8, 4);	/* NFD7_GB -> NFD7_GB */
#endif
}

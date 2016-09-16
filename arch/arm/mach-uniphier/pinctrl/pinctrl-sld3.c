/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "../init.h"
#include "../sg-regs.h"

void uniphier_sld3_pin_init(void)
{
#ifdef CONFIG_NAND_DENALI
	sg_set_pinsel(38, 1, 4, 4);	/* NFALE_GB, NFCLE_GB */
	sg_set_pinsel(39, 1, 4, 4);	/* XNFRYBY0_GB */
	sg_set_pinsel(40, 1, 4, 4);	/* XNFCE0_GB, XNFRE_GB, XNFWE_GB, XNFWP_GB */
	sg_set_pinsel(41, 1, 4, 4);	/* XNFRYBY1_GB, XNFCE1_GB */
	sg_set_pinsel(58, 1, 4, 4);	/* NFD[0-3]_GB */
	sg_set_pinsel(59, 1, 4, 4);	/* NFD[4-7]_GB */
#endif
}

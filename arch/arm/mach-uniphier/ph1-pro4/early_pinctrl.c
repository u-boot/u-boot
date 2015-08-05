/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/io.h>
#include <mach/sg-regs.h>

void early_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_UNIPHIER_SERIAL
	sg_set_pinsel(127, 0);	/* RXD0 -> RXD0 */
	sg_set_pinsel(128, 0);	/* TXD0 -> TXD0 */
	sg_set_pinsel(129, 0);	/* RXD1 -> RXD1 */
	sg_set_pinsel(130, 0);	/* TXD1 -> TXD1 */
	sg_set_pinsel(131, 0);	/* RXD2 -> RXD2 */
	sg_set_pinsel(132, 0);	/* TXD2 -> TXD2 */
	sg_set_pinsel(88, 2);	/* CH6CLK -> RXD3 */
	sg_set_pinsel(89, 2);	/* CH6VAL -> TXD3 */
#endif

	writel(1, SG_LOADPINCTRL);
}

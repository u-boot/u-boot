/*
 * Copyright (C) 2011-2015 Masahiro Yamada <yamada.masahiro@socionext.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <mach/sg-regs.h>

void early_pin_init(void)
{
	/* Comment format:    PAD Name -> Function Name */

#ifdef CONFIG_UNIPHIER_SERIAL
	sg_set_pinsel(63, 0);	/* RXD0 */
	sg_set_pinsel(64, 1);	/* TXD0 */

	sg_set_pinsel(65, 0);	/* RXD1 */
	sg_set_pinsel(66, 1);	/* TXD1 */

	sg_set_pinsel(96, 2);	/* RXD2 */
	sg_set_pinsel(102, 2);	/* TXD2 */
#endif
}

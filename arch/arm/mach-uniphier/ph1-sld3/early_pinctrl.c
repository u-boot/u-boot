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
	sg_set_pinsel(63, 0, 4, 4);	/* RXD0 */
	sg_set_pinsel(64, 1, 4, 4);	/* TXD0 */

	sg_set_pinsel(65, 0, 4, 4);	/* RXD1 */
	sg_set_pinsel(66, 1, 4, 4);	/* TXD1 */

	sg_set_pinsel(96, 2, 4, 4);	/* RXD2 */
	sg_set_pinsel(102, 2, 4, 4);	/* TXD2 */
#endif
}
